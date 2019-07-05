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

void wex::style::apply(wxStyledTextCtrl* stc) const
{
  // Currently the default style is constructed using
  // default constructor.
  // If this is the only style, reset STC.
  if (m_No.empty())
  {
    stc->StyleResetDefault();
  }
  else
  {
    for (const auto& it : m_No)
    {
      stc->StyleSetSpec(it, m_Value);
    }
  }
}

bool wex::style::contains_default_style() const
{
  return (m_No.find(wxSTC_STYLE_DEFAULT) != m_No.end());
}

const std::string wex::style::number() const
{
  return std::accumulate(m_No.begin(), m_No.end(), std::string{}, 
    [](const std::string& a, int b) {return a + std::to_string(b) + ' ';});
}

void wex::style::Set(const pugi::xml_node& node, const std::string& macro)
{
  m_Define = node.attribute("no").value();

  SetNo(
    lexers::get()->apply_macro(m_Define, macro), macro, node);

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  for (tokenizer tkz(node.text().get(), "+"); tkz.has_more_tokens(); )
  {
    // Collect each single field style.
    const auto& single = tkz.get_next_token();

    if (const auto& it = lexers::get()->theme_macros().find(single);
      it != lexers::get()->theme_macros().end())
    {
      wxString value = it->second;

      if (value.Contains("default-font"))
      {
        const wxFont font(config(_("Default font")).get(
          wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));
        
        value.Replace("default-font", 
          "face:" + font.GetFaceName() + ",size:" + std::to_string(font.GetPointSize()));
            
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

      m_Value = (m_Value.empty() ? value: m_Value + "," + value);
    }
    else
    {
      m_Value = (m_Value.empty() ? single: m_Value + "," + single);
    }
  }

  if (m_Value.empty())
  {
    log("empty style") << number() << node;
  }
}

void wex::style::SetNo(
  const std::string& no, 
  const std::string& macro, 
  const pugi::xml_node& node)
{
  m_No.clear();

  // Collect each single no in the vector.
  for (tokenizer tkz(no, ","); tkz.has_more_tokens(); )
  {
    const auto& single = lexers::get()->apply_macro(tkz.get_next_token(), macro);
 
    try
    {
      if (const auto style_no = std::stoi(single);
        style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_No.insert(style_no);
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
