////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wex::style class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <easylogging++.h>

void wex::style::Apply(wxStyledTextCtrl* stc) const
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

bool wex::style::ContainsDefaultStyle() const
{
  return (m_No.find(wxSTC_STYLE_DEFAULT) != m_No.end());
}

const std::string wex::style::GetNo() const
{
  return std::accumulate(m_No.begin(), m_No.end(), std::string{}, 
    [](const std::string& a, int b) {return a + std::to_string(b) + ' ';});
}

void wex::style::Set(const pugi::xml_node& node, const std::string& macro)
{
  m_Define = node.attribute("no").value();

  SetNo(
    lexers::Get()->ApplyMacro(m_Define, macro), macro, node);

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  for (tokenizer tkz(node.text().get(), "+"); tkz.HasMoreTokens(); )
  {
    // Collect each single field style.
    const auto& single = tkz.GetNextToken();

    if (const auto& it = lexers::Get()->GetThemeMacros().find(single);
      it != lexers::Get()->GetThemeMacros().end())
    {
      wxString value = it->second;

      if (value.Contains("default-font"))
      {
        const wxFont font(config(_("Default font")).get(
          wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));
        
        value.Replace("default-font", 
          wxString::Format("face:%s,size:%d",
            font.GetFaceName().c_str(), font.GetPointSize()));
            
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
    log("empty style") << GetNo() << node;
  }
}

void wex::style::SetNo(
  const std::string& no, 
  const std::string& macro, 
  const pugi::xml_node& node)
{
  m_No.clear();

  // Collect each single no in the vector.
  for (tokenizer tkz(no, ","); tkz.HasMoreTokens(); )
  {
    const auto& single = lexers::Get()->ApplyMacro(tkz.GetNextToken(), macro);
 
    try
    {
      if (const auto style_no = std::stoi(single);
        style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_No.insert(style_no);
      }
      else
      {
        VLOG(9) << "illegal style: " << no;
      }
    }
    catch (std::exception& e)
    {
      VLOG(9) << "style exception: " << e.what() << ": " << single;
    }
  }
}
