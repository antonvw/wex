////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wxExStyle class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <numeric>
#include <wx/config.h>
#include <wx/stc/stc.h>
#include <wx/extension/style.h>
#include <wx/extension/lexers.h>
#include <wx/extension/tokenizer.h>

void wxExStyle::Apply(wxStyledTextCtrl* stc) const
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

bool wxExStyle::ContainsDefaultStyle() const
{
  return (m_No.find(wxSTC_STYLE_DEFAULT) != m_No.end());
}

const std::string wxExStyle::GetNo() const
{
  return std::accumulate(m_No.begin(), m_No.end(), std::string{}, 
    [](const std::string& a, int b) {return a + std::to_string(b) + ' ';});
}

void wxExStyle::Set(const pugi::xml_node& node, const std::string& macro)
{
  // TODO: default 0
  SetNo(
    wxExLexers::Get()->ApplyMacro(node.attribute("no").value(), macro),
    macro, node);

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  wxExTokenizer fields(node.text().get(), "+");

  // Collect each single field style.
  while (fields.HasMoreTokens())
  {
    const auto& single = fields.GetNextToken();
    const auto& it = wxExLexers::Get()->GetThemeMacros().find(single);

    if (it != wxExLexers::Get()->GetThemeMacros().end())
    {
      wxString value = it->second;
      
      if (value.Contains("default-font"))
      {
        const wxFont font(wxConfigBase::Get()->ReadObject(_("Default font"), 
          wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));
        
        value.Replace("default-font", 
          wxString::Format("face:%s,size:%d",
            font.GetFaceName().c_str(), font.GetPointSize()));
            
        const wxFontStyle style = font.GetStyle();
            
        if (style == wxFONTSTYLE_ITALIC || style == wxFONTSTYLE_SLANT)
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
    std::cerr << "Empty style: " << GetNo() << " with offset: " 
      << node.offset_debug() << "\n";
  }
}

void wxExStyle::SetNo(const std::string& no, const std::string& macro, 
  const pugi::xml_node& node)
{
  m_No.clear();
  
  wxExTokenizer no_fields(no, ",");

  // Collect each single no in the vector.
  while (no_fields.HasMoreTokens())
  {
    const auto& single = wxExLexers::Get()->ApplyMacro(no_fields.GetNextToken(), macro);
 
    try
    {
      const int style_no = std::stoi(single);
      
      if (style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_No.insert(style_no);
      }
      else
      {
        std::cerr << "Illegal style: " << no << " with offset: " << node.offset_debug() << "\n";
      }
    }
    catch (std::exception& e)
    {
      std::cerr << "Style exception: " << e.what() << "\n";
    }
  }
}
