////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wxExStyle class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <numeric>
#include <wx/config.h>
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>
#include <wx/extension/style.h>
#include <wx/extension/lexers.h>

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
  const auto& it = m_No.find(wxSTC_STYLE_DEFAULT);
  return (it != m_No.end());
}

const std::string wxExStyle::GetNo() const
{
  return std::accumulate(m_No.begin(), m_No.end(), std::string{}, 
    [](const std::string& a, int b) {return a + std::to_string(b) + ' ';});
}

void wxExStyle::Set(const wxXmlNode* node, const std::string& macro)
{
  SetNo(
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0").ToStdString(), macro),
    macro);

  // The style is parsed using the themed macros, and
  // you can specify several styles separated by a + sign.
  wxStringTokenizer fields(node->GetNodeContent().Strip(wxString::both), "+");

  // Collect each single field style.
  while (fields.HasMoreTokens())
  {
    const auto& single = fields.GetNextToken().ToStdString();
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

  if (!IsOk())
  {
    wxLogError("Illegal style: %s on line: %d", 
      m_Value.c_str(), node->GetLineNumber());
  }
}

void wxExStyle::SetNo(const std::string& no, const std::string& macro)
{
  m_No.clear();
  
  wxStringTokenizer no_fields(no, ",");

  // Collect each single no in the vector.
  while (no_fields.HasMoreTokens())
  {
    const auto& single = wxExLexers::Get()->ApplyMacro(no_fields.GetNextToken().ToStdString(), macro);
      
    bool error = true;

    if (wxString(single).IsNumber())
    {
      const int style_no = atoi(single.c_str());
      
      if (style_no >= 0 && style_no <= wxSTC_STYLE_MAX)
      {
        m_No.insert(style_no);
        error = false;
      }
    }
    
    if (error)
    {
      wxLogError("Illegal style: %s", no.c_str());
    }
  }
}
