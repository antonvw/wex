////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <numeric>
#include <wx/stc/stc.h>
#include <wex/style.h>
#include <wex/config.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

void wex::style::apply(wxStyledTextCtrl* stc) const
{
  // Currently the default style is constructed using
  // default constructor.
  // If this is the only style, reset stc.
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

bool wex::style::contains_default_style() const
{
  return (m_no.find(wxSTC_STYLE_DEFAULT) != m_no.end());
}

const std::string wex::style::number() const
{
  return std::accumulate(m_no.begin(), m_no.end(), std::string{}, 
    [](const std::string& a, int b) {return a + std::to_string(b) + ' ';});
}

void wex::style::set(const pugi::xml_node& node, const std::string& macro)
{
  m_define = node.attribute("no").value();

  set_no(
    lexers::get()->apply_macro(m_define, macro), macro, node);

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  for (tokenizer tkz(node.text().get(), "+"); tkz.has_more_tokens(); )
  {
    // Collect each single field style.
    const auto& single = tkz.get_next_token();

    if (const auto& it = lexers::get()->theme_macros().find(single);
      it != lexers::get()->theme_macros().end())
    {
      std::string value(it->second);

      if (value.find("default-font") != std::string::npos)
      {
        const wxFont font(config(_("Default font")).get(
          wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));
        
        replace_all(
          value, 
          "default-font", 
          "face:" + font.GetFaceName() + 
            ",size:" + std::to_string(font.GetPointSize()));
            
        if (const wxFontStyle style = font.GetStyle(); 
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

      m_value = (m_value.empty() ? value: m_value + "," + value);
    }
    else
    {
      m_value = (m_value.empty() ? single: m_value + "," + single);
    }
  }

  if (m_value.empty())
  {
    log("empty style") << number() << node;
  }
}

void wex::style::set_no(
  const std::string& no, 
  const std::string& macro, 
  const pugi::xml_node& node)
{
  m_no.clear();

  // Collect each single no in the vector.
  for (tokenizer tkz(no, ","); tkz.has_more_tokens(); )
  {
    const auto& single = lexers::get()->apply_macro(tkz.get_next_token(), macro);
 
    try
    {
      if (const auto style_no = std::stoi(single);
        style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_no.insert(style_no);
      }
      else
      {
        log("style out of range") << single;
      }
    }
    catch (std::exception& e)
    {
      log(e) << "style:" << single;
    }
  }
}
