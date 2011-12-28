////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.cpp
// Purpose:   Implementation of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

std::map <wxString, wxString> wxExViMacros::m_Macros;

wxExViMacros::wxExViMacros()
  : m_Separator(0x1c)
{
}

void wxExViMacros::Add(const wxString& macro, char c, bool separated)
{
  if (separated) m_Macros[macro] += m_Separator;
  
  m_Macros[macro] += c;
  
  if (separated) m_Macros[macro] += m_Separator;
}

void wxExViMacros::Add(const wxString& macro, const wxString& text)
{
  m_Macros[macro] += m_Separator + text + m_Separator;
}

void wxExViMacros::AddSeparator(const wxString& macro)
{
  m_Macros[macro] += m_Separator;
}

void wxExViMacros::Clear(const wxString& macro)
{
  m_Macros[macro].clear();
}

const wxString wxExViMacros::Get(const wxString& macro) const
{
  std::map<wxString, wxString>::const_iterator it = m_Macros.find(macro);
    
  if (it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    return wxEmptyString;
  }
}

const wxArrayString wxExViMacros::Get() const
{
  wxArrayString as;
    
  for (
    std::map<wxString, wxString>::const_iterator it = m_Macros.begin();
    it != m_Macros.end();
    ++it)
  {
    // Add only if we have content.
    if (!it->second.empty())
    {
      as.Add(it->first);
    }
  }
   
  return as;
}

const wxFileName wxExViMacros::GetFileName()
{
  return wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "macros.xml");
}

bool wxExViMacros::IsAvailable(const wxString& macro) const
{
  if (macro.empty())
  {
    return !m_Macros.empty();
  }
  else
  {
    return !Get(macro).empty();
  }
}

bool wxExViMacros::Load(wxXmlDocument& doc)
{
  // This test is to prevent showing an error if the macro file does not exist,
  // as this is not required.
  if (!GetFileName().FileExists())
  {
    return false;
  } 
  
  if (!doc.Load(
    GetFileName().GetFullPath(),
    "UTF-16"))
  {
    return false;
  }
  
  return true;
}

void wxExViMacros::LoadDocument()
{
  wxXmlDocument doc;
  
  if (Load(doc))
  {
    wxXmlNode* root = doc.GetRoot();
    wxXmlNode* child = root->GetChildren();
  
    while (child)
    {
      m_Macros[child->GetName()] = child->GetNodeContent();
      child = child->GetNext();
    }
  }
}

void wxExViMacros::SaveDocument()
{
  wxXmlDocument doc;
  
  if (!Load(doc))
  {
    return;
  }
  
  wxXmlNode* root = doc.GetRoot();
  wxXmlNode* child;
  
  while (child = root->GetChildren())
  {
    root->RemoveChild(child);
    delete child;
  }
 
  for (
    std::map<wxString, wxString>::reverse_iterator it = m_Macros.rbegin();
    it != m_Macros.rend();
    ++it)
  {
    wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, it->first);
    wxXmlNode* content = new wxXmlNode(element, wxXML_TEXT_NODE, it->first, it->second);
  }
  
  doc.Save(GetFileName().GetFullPath());
}

#endif // wxUSE_GUI
