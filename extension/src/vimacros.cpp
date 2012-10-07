////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.cpp
// Purpose:   Implementation of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stdpaths.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/utils.h>
#include <wx/extension/vimacros.h>
#include <wx/extension/ex.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

bool wxExViMacros::m_IsExpand = false;
bool wxExViMacros::m_IsModified = false;
bool wxExViMacros::m_IsPlayback = false;

std::map <wxString, std::vector< wxString > > wxExViMacros::m_Macros;
std::map <wxString, wxExVariable > wxExViMacros::m_Variables;

wxExViMacros::wxExViMacros()
  : m_IsRecording(false)
{
}

void wxExViMacros::AskForInput()
{
  for (std::map<wxString, wxExVariable >::iterator it = 
    m_Variables.begin();
    it != m_Variables.end();
    ++it)
  {
    it->second.AskForInput();
  }
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
  
    // Encode control characters, and whitespace.
    if (iscntrl(c) || isspace(c))
    {
      encoded = true;
      return wxString::Format("%d", c);
    }
  }

  return text;  
}

bool wxExViMacros::Expand(wxExEx* ex, const wxString& variable)
{
  std::map<wxString, wxExVariable>::iterator it = m_Variables.find(variable);
  
  bool ok;
    
  if (it == m_Variables.end())
  {
    wxExVariable var(variable);
    m_Variables.insert(std::make_pair(variable, var));
    wxLogStatus(_("Added variable") + ": "  +  variable);
    
    ok = var.Expand(ex);
  
    if (var.IsModified())
    {
      m_IsModified = true;
    }
  }
  else
  {
    ok = it->second.Expand(ex);
  
    if (it->second.IsModified())
    {
      m_IsModified = true;
    }
  }
  
  if (!ok)
  {
    wxLogStatus(_("Could not expand variable") + ": "  +  variable);
  }
  
  return ok;
}  

bool wxExViMacros::Expand(wxExEx* ex, const wxString& variable, wxString& value)
{
  std::map<wxString, wxExVariable>::iterator it = m_Variables.find(variable);
    
  bool ok;
    
  if (it == m_Variables.end())
  {
    wxExVariable var(variable);
    m_Variables.insert(std::make_pair(variable, var));
    wxLogStatus(_("Added variable") + ": "  +  variable);
    
    ok = var.Expand(ex, value);
  
    // If we are expanding, one input is enough.    
    if (m_IsExpand)
    {
      var.SkipInput();
    }
    
    if (var.IsModified())
    {
      m_IsModified = true;
    }
  }
  else
  {
    ok = it->second.Expand(ex, value);

    // If we are expanding, one input is enough.    
    if (m_IsExpand)
    {
      it->second.SkipInput();
    }
  
    if (it->second.IsModified())
    {
      m_IsModified = true;
    }
  }
  
  if (!ok)
  {
    wxLogStatus(_("Could not expand variable") + ": "  +  variable);
  }
  
  return ok;
}

bool wxExViMacros::ExpandTemplate(
  wxExEx* ex, const wxExVariable& v, wxString& expanded)
{
  if (!m_IsExpand)
  {
    m_IsExpand = true;
    AskForInput();
  }

  // Read the file (file name is in m_Value), expand
  // all macro variables in it, and set expanded.
  const wxFileName filename(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + v.GetValue());

  wxFileInputStream input(filename.GetFullPath());
  
  if (!input.IsOk())
  {
    wxLogError("Could not open template file: " + filename.GetFullPath());
    return false;
  }
  
  wxTextInputStream text(input);
  
  while (input.IsOk() && !input.Eof()) 
  {
    const wxChar c = text.GetChar();
    
    if (c != '@')
    {
      expanded += c;
    }
    else
    {
      wxString variable;
      bool completed = false;
      
      while (input.IsOk() && !input.Eof() && !completed) 
      {
        const wxChar c = text.GetChar();
    
        if (c != '@')
        {
          variable += c;
        }
        else
        {
          completed = true;
        }
      }
      
      if (!completed)
      {
        return false;
      }
      
      // Prevent recursion.
      if (variable == v.GetName())
      {
        return false;
      }
      
      wxString value;
      
      if (!Expand(ex, variable, value))
      {
        return false;
      }
      
      expanded += value;
    }
  }
  
  m_IsExpand = false;

  // Set back to normal value.  
  AskForInput();
    
  return true;
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
    as.Add(it->first);
  }
   
  return as;
}

const std::vector< wxString > wxExViMacros::Get(const wxString& macro) const
{
  std::map<wxString, std::vector< wxString > >::const_iterator it = 
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

bool wxExViMacros::LoadDocument()
{
  wxXmlDocument doc;
  
  if (!Load(doc))
  {
    return false;
  }
  
  // If modified is true, then you did not save previous
  // recordings.
  // We assume that this is your choice, so we reset the member.
  m_IsModified = false;
  
  m_Macros.clear();
  m_Variables.clear();
  
  wxXmlNode* root = doc.GetRoot();
  wxXmlNode* child = root->GetChildren();
  
  while (child)
  {
    if (child->GetName() == "macro")
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
      
      std::map<wxString, std::vector< wxString >>::const_iterator it = 
        m_Macros.find(child->GetAttribute("name"));
    
      if (it != m_Macros.end())
      {
        wxLogError("Duplicate macro: %s on line: %d", 
          child->GetAttribute("name"),
          child->GetLineNumber());
      }
      else
      {
        m_Macros.insert(std::make_pair(child->GetAttribute("name"), v));
      }
    }
    else if (child->GetName() == "variable")
    {
      wxExVariable variable(child);
      
      std::map<wxString, wxExVariable>::const_iterator it = m_Variables.find(variable.GetName());
    
      if (it != m_Variables.end())
      {
        wxLogError("Duplicate variable: %s on line: %d", 
         variable.GetName(),
         child->GetLineNumber());
      }
      else
      {
        m_Variables.insert(std::make_pair(variable.GetName(), variable));
      }
    }
      
    child = child->GetNext();
  }
  
  return true;
}

bool wxExViMacros::Playback(wxExEx* ex, const wxString& macro, int repeat)
{
  if (!IsRecorded(macro) || macro.empty())
  {
    wxLogStatus(_("Unknown macro") + ": "  +  macro);
    return false;
  }
  
  if (m_IsPlayback && macro == m_Macro)
  {
    wxLogStatus(_("Already playing back"));
    return false;
  }

  if (repeat <= 0)
  {
    return false;
  }
  
  ex->GetSTC()->BeginUndoAction();
  
  bool stop = false;
  
  m_IsPlayback = true;
  
  m_Macro = macro;
  
  for (std::map<wxString, wxExVariable >::iterator it = 
    m_Variables.begin();
    it != m_Variables.end();
    ++it)
  {
    it->second.AskForInput();
  }
    
  for (int i = 0; i < repeat; i++)
  {
    for (
      std::vector<wxString>::const_iterator it = m_Macros[macro].begin();
      it != m_Macros[macro].end() && !stop;
      ++it)
    { 
      stop = !ex->Command(*it);
      
      if (stop)
      {
        wxLogStatus(_("Macro aborted at '") + *it + "'");
      }
    }
  }

  ex->GetSTC()->EndUndoAction();

  if (!stop)
  {
    wxLogStatus(_("Macro played back"));
  }
  
  m_IsPlayback = false;
  
  return !stop;
}

void wxExViMacros::Record(const wxString& text, bool new_command)
{
  if (!m_IsRecording)
  {
    return;
  }
  
  m_IsModified = true;
  
  if (new_command) 
  {
    m_Macros[m_Macro].push_back(text);
  }
  else
  {
    if (m_Macros[m_Macro].empty())
    {
      m_Macros[m_Macro].push_back(wxEmptyString);
    }
    
    m_Macros[m_Macro].back() += text;
  }
}

bool wxExViMacros::SaveDocument(bool only_if_modified)
{
  if (!m_IsModified && only_if_modified)
  {
    return false;
  }
  
  wxXmlDocument doc;
  
  if (!Load(doc))
  {
    return false;
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
      
      wxXmlNode* cmd = new wxXmlNode(element, wxXML_ELEMENT_NODE, "command");
      new wxXmlNode(cmd, wxXML_TEXT_NODE, "", contents);
        
      if (encoded)
      {
        cmd->AddAttribute("encoded", "true");
      }
    }
  }
  
  for (
    std::map< wxString, wxExVariable >::reverse_iterator it2 = 
      m_Variables.rbegin();
    it2 != m_Variables.rend();
    ++it2)
  {
    wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, "variable");
    it2->second.Save(element);
  }
  
  const bool ok = doc.Save(GetFileName().GetFullPath());
  
  if (ok)
  {
    m_IsModified = false;
  }

  return ok;
}

void wxExViMacros::StartRecording(const wxString& macro)
{
  if (m_IsRecording || macro.empty())
  {
    return;
  }
  
  m_IsRecording = true;
  
  if (macro.size() == 1)
  {
    // We only use lower case macro's, to be able to
    // append to them using qA.
    m_Macro = macro.Lower();
  
    // Clear macro if it is lower case
    // (otherwise append to the macro).
    if (wxIslower(macro[0]))
    {
      m_Macros[m_Macro].clear();
    }
  }
  else
  {
    m_Macro = macro;
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
  
  if (!Get(m_Macro).empty())
  {
    wxLogStatus(wxString::Format(_("Macro '%s' is recorded"), m_Macro.c_str()));
  }
  else
  {
    wxLogStatus(_("Ready"));
  }
}

#endif // wxUSE_GUI
