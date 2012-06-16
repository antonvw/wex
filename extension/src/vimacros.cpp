////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.cpp
// Purpose:   Implementation of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/extension/vimacros.h>
#include <wx/extension/ex.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

bool wxExViMacros::m_IsModified = false;

std::map <wxString, std::vector< wxString > > wxExViMacros::m_Macros;
std::map <wxString, wxExVariable > wxExViMacros::m_Variables;

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
    
  if (it == m_Variables.end())
  {
    wxLogStatus(_("Unknown macro") + ": "  +  variable);
    return false;
  }
  
  return it->second.Expand(m_IsPlayback, ex);
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
      
      m_Macros[child->GetAttribute("name")] = v;
    }
    else if (child->GetName() == "variable")
    {
      wxExVariable variable(child);
      m_Variables.insert(std::make_pair(child->GetName(), variable));
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
  
  ex->GetSTC()->BeginUndoAction();
  
  bool stop = false;
  
  m_IsPlayback = true;
  
  // Clear all input variables values.
  for (
    std::map<wxString, wxExVariable>::iterator it = 
      m_Variables.begin();
    it != m_Variables.end();
    ++it)
  {
    it->second.Clear();
  }
  
  m_Macro = macro;
  
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

  if (!stop && repeat > 0)
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

/// Several types of variables are supported.
/// See xml file.
enum
{
  VARIABLE_UNKNOWN,        ///< variable is not known
  VARIABLE_BUILTIN,        ///< a builtin variable
  VARIABLE_ENVIRONMENT,    ///< an environment variable
  VARIABLE_INPUT,          ///< input once from user
  VARIABLE_INPUT_COMMENT,  ///< input once from user, used for comments
  VARIABLE_XML             ///< value from xml file
};

wxExVariable::wxExVariable()
{
}
  
wxExVariable::wxExVariable(const wxXmlNode* node)
  : m_Type(VARIABLE_XML)
{
  const wxString type = node->GetAttribute("type");
      
  if (type == "BUILTIN")
  {
    m_Type = VARIABLE_BUILTIN;
  }
  else if (type == "ENVIRONMENT")
  {
    m_Type = VARIABLE_ENVIRONMENT;
  }
  else if (type == "INPUT")
  {
    m_Type = VARIABLE_INPUT;
  }
  else if (type == "INPUT-COMMENT")
  {
    m_Type = VARIABLE_INPUT_COMMENT;
  }
      
  m_Prefix = node->GetAttribute("prefix");
  m_Value = node->GetNodeContent().Strip(wxString::both);
}

void wxExVariable::Clear()
{
  if (m_Type == VARIABLE_INPUT)
  {
    m_Value.clear();
  }
}

bool wxExVariable::Expand(bool playback, wxExEx* ex)
{
  wxString text;
  
  switch (m_Type)
  {
    case VARIABLE_UNKNOWN:
      return false;
      break;
      
    case VARIABLE_BUILTIN:
      if (!ExpandBuiltIn(ex, text))
      {
        return false;
      }
      break;
      
    case VARIABLE_ENVIRONMENT:
      if (!wxGetEnv(m_Name, &text))
      {
        return false;
      }
      break;
      
    case VARIABLE_INPUT:
      if (!ExpandInput(playback, text))
      {
        return false;
      }
      break;
      
    case VARIABLE_INPUT_COMMENT:
      // First expand variable.
      if (!ExpandInput(playback, text))
      {
        return false;
      }
      
      // Then make comment out of it, using variable as a prefix.
      text = ex->GetSTC()->GetLexer().MakeComment(m_Prefix, text);
      break;
      
    case VARIABLE_XML:
      text = m_Value;
      break;
      
    default: wxFAIL; break;
  }
  
  if (ex->GetSTC()->GetReadOnly() || ex->GetSTC()->HexMode())
  {
    return false;
  }
  
  ex->GetSTC()->AddText(text);
    
  return true;
}

bool wxExVariable::ExpandBuiltIn(wxExEx* ex, wxString& expanded) const
{
  if (m_Name == "CB")
  {
    expanded = ex->GetSTC()->GetLexer().GetCommentBegin();
  }
  else if (m_Name == "CE")
  {
    expanded = ex->GetSTC()->GetLexer().GetCommentEnd();
  }
  else if (m_Name == "CL")
  {
    expanded = ex->GetSTC()->GetLexer().MakeComment(wxEmptyString, false);
  }
  else if (m_Name == "DATE")
  {
    expanded = wxDateTime::Now().FormatISODate();
  }
  else if (m_Name == "DATETIME")
  {
    expanded = wxDateTime::Now().FormatISOCombined(' ');
  }
  else if (m_Name == "FILENAME")
  {
    expanded = ex->GetSTC()->GetFileName().GetFullName();
  }
  else if (m_Name == "NL")
  {
    expanded = ex->GetSTC()->GetEOL();
  }
  else if (m_Name == "TIME")
  {
    expanded = wxDateTime::Now().FormatISOTime();
  }
  else if (m_Name == "YEAR")
  {
    expanded = wxDateTime::Now().Format("%Y");
  }
  else
  {
    return false;
  }
  
  return true;
}

bool wxExVariable::ExpandInput(bool playback, wxString& expanded)
{
  if (!playback || m_Value.empty())
  {
    const wxString value = wxGetTextFromUser(
      m_Name,
      wxGetTextFromUserPromptStr,
      m_Value);
         
    if (value.empty())
    {
      return false;
    }
          
    expanded = value;
    m_Value = value;
  }
  else
  {
    expanded = m_Value;
  }
  
  return true;
}

void wxExVariable::Save(wxXmlNode* node) const
{
  node->AddAttribute("name", m_Name);
  
  if (m_Prefix.empty())
  {
    node->AddAttribute("prefix", m_Prefix);
  }
    
  switch (m_Type)
  {
    case VARIABLE_UNKNOWN:
      wxFAIL;
      break;
      
    case VARIABLE_BUILTIN:
      node->AddAttribute("type", "BUILTIN");
      break;
      
    case VARIABLE_ENVIRONMENT:
      node->AddAttribute("type", "ENVIRONMENT");
      break;
      
    case VARIABLE_INPUT:
      node->AddAttribute("type", "INPUT");
      break;
      
    case VARIABLE_INPUT_COMMENT:
      node->AddAttribute("type", "INPUT-COMMENT");
      break;
      
    case VARIABLE_XML:
      new wxXmlNode(node, wxXML_TEXT_NODE, "", m_Value);
      break;
      
    default: wxFAIL; break;
  }
} 

#endif // wxUSE_GUI
