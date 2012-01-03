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
#include <wx/extension/vimacros.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

std::map <wxString, std::vector< wxString > > wxExViMacros::m_Macros;

wxExViMacros::wxExViMacros()
  : m_IsRecording(false)
  , m_IsPlayback(false)
{
}

const wxString wxExViMacros::Decode(const wxString& text)
{
  long c;
  
  if (text.ToLong(&c))
  {
    return char(c);
  }
  
  return text;
}
    
const wxString wxExViMacros::Encode(const wxString& text, bool& encoded)
{
  if (text.length() == 1)
  {
    int c = text[0];
  
    if (iscntrl(c))
    {
      encoded = true;
      return wxString::Format("%d", c);
    }
  }

  return text;  
}

const std::vector< wxString > wxExViMacros::Get(const wxString& macro) const
{
  std::map<wxString, std::vector< wxString >>::const_iterator it = 
    m_Macros.find(macro);
    
  if (it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    std::vector<wxString> empty;
    return empty;
  }
}

const wxArrayString wxExViMacros::Get() const
{
  wxArrayString as;
    
  for (
    std::map<wxString, std::vector<wxString> >::const_iterator it = 
      m_Macros.begin();
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

bool wxExViMacros::IsRecorded(const wxString& macro) const
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
  
  if (!doc.Load(GetFileName().GetFullPath()))
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
      std::vector<wxString> v;
      
      wxXmlNode* command = child->GetChildren();
  
      while (command)
      {
        if (command->GetAttribute("encoded", "false") == "true")
        {
          v.push_back(Decode(command->GetNodeContent()));
        }
        else
        {
          v.push_back(command->GetNodeContent());
        }
        
        command = command->GetNext();
      }
      
      m_Macros[child->GetAttribute("name")] = v;
      
      child = child->GetNext();
    }
  }
}

bool wxExViMacros::Playback(wxExVi* vi, const wxString& macro, int repeat)
{
  if (!IsRecorded(macro))
  {
    wxLogStatus(_("Unknown macro"));
    return false;
  }
  
  vi->GetSTC()->BeginUndoAction();
  
  bool stop = false;
  
  m_IsPlayback = true;
    
  for (int i = 0; i < repeat; i++)
  {
    for (
      std::vector<wxString>::const_iterator it = m_Macros[macro].begin();
      it != m_Macros[macro].end() && !stop;
      ++it)
    { 
      const wxString command(*it);

      if (command == "/" || command == "?")
      {
        ++it;
        
        if (it != m_Macros[macro].end())
        {
          stop = !vi->FindNext(*it, command == "/");
        }
      }
      else if (command == ":")
      {
        ++it;
        
        if (it != m_Macros[macro].end())
        {
          stop = !vi->ExecCommand(*it);
        }
      }
      else if (!command.empty())
      {
        stop = !vi->Command(command);
      }
    }
  }

  vi->GetSTC()->EndUndoAction();
  
  wxLogStatus(!stop ? _("Macro played back"): _("Macro aborted"));
  
  m_IsPlayback = false;
  
  return !stop;
}

void wxExViMacros::Record(const wxString& text)
{
  if (
    !m_Macros[m_Macro].empty() &&
     m_Macros[m_Macro].back().empty())
  {
    m_Macros[m_Macro].back() += text;
  }
  else
  {
    m_Macros[m_Macro].push_back(text);
  }
  
  RecordNew();
}

void wxExViMacros::Record(char c, bool new_command)
{
  if (new_command) 
  {
    m_Macros[m_Macro].push_back(c);
    
    RecordNew();
  }
  else
  {
    if (m_Macros[m_Macro].empty())
    {
      RecordNew();
    }
    
    m_Macros[m_Macro].back() += c;
  }
}

void wxExViMacros::RecordNew()
{
  m_Macros[m_Macro].push_back(wxEmptyString);
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
    std::map<wxString, std::vector<wxString> >::reverse_iterator it = 
      m_Macros.rbegin();
    it != m_Macros.rend();
    ++it)
  {
    wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, "macro");
    element->AddAttribute("name", it->first);
    
    for (
      std::vector<wxString>::reverse_iterator it2 = it->second.rbegin();
      it2 != it->second.rend();
      ++it2)
    { 
      bool encoded = false;  
      const wxString contents(Encode(*it2, encoded));
      
      if (!contents.empty())
      {
        wxXmlNode* cmd = new wxXmlNode(element, wxXML_ELEMENT_NODE, "command");
        wxXmlNode* content = new wxXmlNode(cmd, wxXML_TEXT_NODE, "", contents);
        
        if (encoded)
        {
          cmd->AddAttribute("encoded", "true");
        }
      }
    }
  }
  
  doc.Save(GetFileName().GetFullPath());
}

void wxExViMacros::StartRecording(const wxString& macro)
{
  if (m_IsRecording)
  {
    return;
  }
  
  m_IsRecording = true;
  
  // We only use lower case macro's, to be able to
  // append to them using qA.
  m_Macro = macro.Lower();
  
  // Only clear macro if starts with lower case,
  // otherwise append to the macro.
  if (wxIslower(macro[0]))
  {
    m_Macros[m_Macro].clear();
  }
  
  wxLogStatus(_("Macro recording"));
}

void wxExViMacros::StopRecording()
{
  if (!m_IsRecording)
  {
    return;
  }
  
  m_IsRecording = false;
  wxLogStatus(_("Macro is recorded"));
}

#endif // wxUSE_GUI
