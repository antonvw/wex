////////////////////////////////////////////////////////////////////////////////
// Name:      style.cpp
// Purpose:   Implementation of wxExStyle class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/extension/style.h>
#include <wx/extension/lexers.h>

wxExStyle::wxExStyle()
  : m_No()
  , m_Value()
{
}
  
wxExStyle::wxExStyle(const wxXmlNode* node)
{
  Set(node);
}

wxExStyle::wxExStyle(const wxString& no, const wxString& value)
  : m_Value(value)
{
  SetNo(no);
}

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
    for (
      auto it = m_No.begin();
      it != m_No.end();
      ++it)
    {
      stc->StyleSetSpec(*it, m_Value);
    }
  }
}

bool wxExStyle::ContainsDefaultStyle() const
{
  const auto it = m_No.find(wxSTC_STYLE_DEFAULT);
  return (it != m_No.end());
}

bool wxExStyle::IsOk() const
{
  return !m_No.empty() && !m_Value.empty();
}

void wxExStyle::Set(const wxXmlNode* node)
{
  SetNo(wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")));

  m_Value = node->GetNodeContent().Strip(wxString::both);

  const auto it = 
    wxExLexers::Get()->GetThemeMacros().find(m_Value);

  if (it != wxExLexers::Get()->GetThemeMacros().end())
  {
    wxString value = it->second;
    
    if (value.Contains("default-font"))
    {
      const wxFont font(wxConfigBase::Get()->ReadObject(_("Default font"), 
        wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));
      
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
    
    m_Value = value;
  }

  if (!IsOk())
  {
    wxLogError(_("Illegal style: %s on line: %d"), 
      m_Value.c_str(), node->GetLineNumber());
  }
}

void wxExStyle::SetNo(const wxString& no)
{
  m_No.clear();
  
  wxStringTokenizer no_fields(no, ",");

  // Collect each single no in the vector.
  while (no_fields.HasMoreTokens())
  {
    const wxString single = 
      wxExLexers::Get()->ApplyMacro(no_fields.GetNextToken());

    if (single.IsNumber())
    {
      m_No.insert(atoi(single.c_str()));
    }
    else
    {
      wxLogError(_("Illegal style: %s"), no.c_str());
    }
  }
}
