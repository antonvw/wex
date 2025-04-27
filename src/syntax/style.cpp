////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/style.h>
#include <wx/stc/stc.h>

#include <charconv>

namespace wex
{
bool check_style_spec(const std::string& spec, const std::string& colour)
{
  if (regex r(colour + ":(.*)"); r.match(spec) > 0)
  {
    if (!wxColour(r[0]).IsOk())
    {
      log("style " + colour + " colour") << r[0];
      return false;
    }
  }

  return true;
}

} // namespace wex

bool wex::style::apply(wxStyledTextCtrl* stc) const
{
  if (stc->GetParent() == nullptr)
  {
    return false;
  }

  // Currently, the default style is constructed using
  // default constructor.
  // If this is the only style, reset stc.
  if (m_no.empty())
  {
    if (!lexers::get()->get_lexers().empty())
    {
      stc->StyleResetDefault();
    }
  }
  else
  {
    if (const auto& tok(boost::tokenizer<boost::char_separator<char>>(
          m_value,
          boost::char_separator<char>(",")));
        !std::ranges::all_of(
          tok,
          [](const auto& it)
          {
            return check_style_spec(it, "back") && check_style_spec(it, "fore");
          }))
    {
      return false;
    }

    for (const auto& it : m_no)
    {
      stc->StyleSetSpec(it, m_value);
    }
  }

  return true;
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

wxFont wex::style::default_font() const
{
  return config(_("stc.Default font"))
    .get(wxFont(
#ifdef __WXOSX__
      14,
#else
      12,
#endif
      wxFONTFAMILY_TELETYPE,
      wxFONTSTYLE_NORMAL,
      wxFONTWEIGHT_NORMAL));
}

int wex::style::default_font_size() const
{
  return default_font().GetPointSize();
}

int wex::style::number() const
{
  return m_no.empty() ? -1 : *m_no.begin();
}

void wex::style::set(const pugi::xml_node& node, const std::string& macro)
{
  m_define = node.attribute("no").value();

  set_no(lexers::get()->apply_macro(m_define, macro), macro, node);

  const auto text(std::string(node.text().get()));
  const auto font(default_font());
  const auto font_size(std::to_string(default_font_size()));

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

      if (value.contains("default-font"))
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
