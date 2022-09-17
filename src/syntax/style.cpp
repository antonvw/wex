////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/style.h>
#include <wx/stc/stc.h>

#include <charconv>

void wex::style::apply(wxStyledTextCtrl* stc) const
{
  // Currently the default style is constructed using
  // default constructor.
  // If this is the only style, reset stc.
  if (stc->GetParent() != nullptr)
  {
    if (m_no.empty())
    {
      stc->StyleResetDefault();
    }
    else
    {
      for (const auto& it : m_no)
      {
        stc->StyleSetSpec(it, m_value);
      }
    }
  }
}

void wex::style::clear()
{
  m_define.clear();
  m_no.clear();
  m_value.clear();
}

bool wex::style::contains_default_style() const
{
  return (m_no.find(wxSTC_STYLE_DEFAULT) != m_no.end());
}

int wex::style::number() const
{
  return m_no.empty() ? -1 : *m_no.begin();
}

void wex::style::set(const pugi::xml_node& node, const std::string& macro)
{
  m_define = node.attribute("no").value();

  set_no(lexers::get()->apply_macro(m_define, macro), macro, node);

  const std::string text(std::string(node.text().get()));
  const auto        font(config(_("stc.Default font"))
                    .get(wxFont(
                      12,
                      wxFONTFAMILY_DEFAULT,
                      wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_NORMAL)));
  const auto        font_size(std::to_string(font.GetPointSize()));

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         text,
         boost::char_separator<char>("+")))
  {
    // Collect each single field style.
    const auto& single = it;

    if (const auto& it = lexers::get()->theme_macros().find(single);
        it != lexers::get()->theme_macros().end())
    {
      std::string value(it->second);

      if (value.find("default-font") != std::string::npos)
      {
        boost::algorithm::replace_all(
          value,
          "default-font",
          "face:" + font.GetFaceName() + ",size:" + font_size);

        if (const auto style = font.GetStyle();
            style == wxFONTSTYLE_ITALIC || style == wxFONTSTYLE_SLANT)
        {
          value += ",italic";
        }

        if (font.GetWeight() == wxFONTWEIGHT_BOLD)
        {
          value += ",bold";
        }

        if (font.GetUnderlined())
        {
          value += ",underline";
        }
      }

      m_value = (m_value.empty() ? value : m_value + "," + value);
    }
    else
    {
      m_value = (m_value.empty() ? single : m_value + "," + single);
    }
  }

  if (m_value.empty())
  {
    log("empty style") << number() << node;
  }
}

void wex::style::set_no(
  const std::string&    no,
  const std::string&    macro,
  const pugi::xml_node& node)
{
  m_no.clear();

  // Collect each single no in the vector.
  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         no,
         boost::char_separator<char>(",")))
  {
    const auto& single = lexers::get()->apply_macro(it, macro);

    try
    {
      if (int style_no = 0; std::from_chars(
                              single.data(),
                              single.data() + single.size(),
                              style_no)
                                .ec == std::errc() &&
                            style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_no.insert(style_no);
      }
      else
      {
        log("invalid style") << single;
      }
    }
    catch (std::exception& e)
    {
      log(e) << "style:" << single;
    }
  }
}
