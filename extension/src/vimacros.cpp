////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.cpp
// Purpose:   Implementation of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <fstream>
#include <numeric>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vimacros.h>
#include <wx/extension/ex.h>
#include <wx/extension/frame.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

bool wxExViMacros::m_IsExpand = false;
bool wxExViMacros::m_IsModified = false;
bool wxExViMacros::m_IsPlayback = false;
bool wxExViMacros::m_IsRecording = false;
std::string wxExViMacros::m_Macro;

std::map <std::string, std::string> wxExViMacros::m_Abbreviations;
std::map <std::string, std::vector< std::string > > wxExViMacros::m_Macros;
std::map <std::string, wxExVariable > wxExViMacros::m_Variables;

void wxExViMacros::AskForInput()
{
  for (auto& it : m_Variables)
  {
    it.second.AskForInput();
  }
}

const std::string wxExViMacros::Decode(const std::string& text)
{
  std::string output;

  for (size_t i = 0; i < text.length(); i++)
  {
    if (
      i + 1 < text.length() &&
      text[i]     == '$' &&
      text[i + 1] == '!')
    {
      int skip = 0;
      wxString number;
    
      if (
        i + 3 < text.length() &&
        isdigit(text[i + 2]) && 
        text[i + 3] == '!')
      {
        skip  = 3;
        number = text.substr(i + 2, 1);
      }
      else if (
        i + 4 < text.length() &&
        isdigit(text[i + 2]) &&
        isdigit(text[i + 3]) &&
        text[i + 4] == '!')
      {
        skip = 4;
        number = text.substr(i + 2, 2);
      }
      
      if (!number.empty())
      {
        output += wxChar(atoi(number));
        i += skip;
      }
    }
    else
    {
      output += text[i];
    }
  }
  
  return output;
}

const std::string wxExViMacros::Encode(const std::string& text)
{
  std::string output;
  
  for (size_t i = 0; i < text.length(); i++)
  {
    const int c = text[i];
  
    // Encode control and whitespace characters.
    if (iscntrl(c) || isspace(c))
    {
      output += wxString::Format("$!%d!", c);
    }
    else
    {
      output += text[i];
    }
  }

  return output;
}

bool wxExViMacros::Expand(wxExEx* ex, const std::string& variable)
{
  const auto& it = m_Variables.find(variable);
  
  bool ok;
    
  if (it == m_Variables.end())
  {
    auto ret = m_Variables.insert({variable, wxExVariable(variable)});
      
    wxLogStatus(_("Added variable") + ": "  +  variable);
    
    ok = ret.first->second.Expand(ex);
  
    if (ret.first->second.IsModified())
    {
      m_IsModified = true;
    }
    
    // If ok is false, this is because expansion dialog was cancelled,
    // no need to show log status message.
  }
  else
  {
    ok = it->second.Expand(ex);
  
    if (it->second.IsModified())
    {
      m_IsModified = true;
    }

    // Now only show log status if this is no input variable,
    // as it was cancelled in that case.    
    if (!ok && !it->second.IsInput())
    {
      wxLogStatus(_("Could not expand variable") + ": "  +  variable);
    }
  }

  if (ok)
  {
    wxLogStatus(_("Variable expanded"));
  
    if (!m_IsRecording)
    {
      m_Macro = variable;
    }
  }
  
  return ok;
}  

bool wxExViMacros::Expand(wxExEx* ex, const std::string& variable, std::string& value)
{
  const auto& it = m_Variables.find(variable);
    
  bool ok;
    
  if (it == m_Variables.end())
  {
    auto ret = m_Variables.insert({variable, wxExVariable(variable)});
      
    wxLogStatus(_("Added variable") + ": "  +  variable);
    
    ok = ret.first->second.Expand(ex, value);
  
    // If we are expanding, one input is enough.    
    if (m_IsExpand)
    {
      ret.first->second.SkipInput();
    }
    
    if (ret.first->second.IsModified())
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
  else 
  {
    wxLogStatus(_("Variable expanded"));
  
    if (!m_IsRecording)
    {
      m_Macro = variable;
    }
  }
  
  return ok;
}

bool wxExViMacros::ExpandTemplate(
  wxExEx* ex, const wxExVariable& v, std::string& expanded)
{
  if (v.GetValue().empty())
  {
    return false;
  }
  
  if (!m_IsExpand)
  {
    m_IsExpand = true;
    AskForInput();
  }

  // Read the file (file name is in v.GetValue()), expand
  // all macro variables in it, and set expanded.
  const wxExFileName filename(wxExConfigDir(), v.GetValue());

  std::ifstream ifs(filename.GetFullPath());
  
  if (!ifs.is_open())
  {
    wxLogError("Could not open template file: %s", filename.GetFullPath().c_str());
    return false;
  }

  // Keep current macro, in case you cancel expanding,
  // this one is restored.
  std::string macro = m_Macro;
  
  while (ifs.good() && !ifs.eof()) 
  {
    const char c = ifs.get();
    
    if (c != '@')
    {
      expanded += c;
    }
    else
    {
      std::string variable;
      bool completed = false;
      
      while (ifs.good() && !ifs.eof() && !completed) 
      {
        const char c = ifs.get();
    
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
        m_Macro = macro;
        return false;
      }
      
      // Prevent recursion.
      if (variable == v.GetName())
      {
        m_Macro = macro;
        return false;
      }
      
      std::string value;
      
      if (!Expand(ex, variable, value))
      {
        m_Macro = macro;
        return false;
      }
      
      expanded += value;
    }
  }
  
  m_IsExpand = false;

  // Set back to normal value.  
  AskForInput();
    
  if (!m_IsRecording)
  {
    m_Macro = v.GetName();
  }
    
  return true;
}

const std::vector< std::string > wxExViMacros::Get() const
{
  std::vector< std::string > v;
    
  for (const auto& it : m_Macros)
  {
    if (it.first.size() > 1)
    {
      v.emplace_back(it.first);
    }
  }
   
  for (const auto& it : m_Variables)
  {
    v.emplace_back(it.first);
  }
  
  std::sort(v.begin(), v.end());
  
  return v;
}

const std::vector< std::string > wxExViMacros::Get(
  const std::string& macro) const
{
  const auto& it = m_Macros.find(macro);
    
  if (it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    const auto& it = m_Variables.find(macro);
    std::vector<std::string> v;
    
    if (it != m_Variables.end())
    {
      v.emplace_back(it->second.GetValue());
    }
  
    return v;
  }
}

const wxFileName wxExViMacros::GetFileName()
{
  return wxFileName(wxExConfigDir(), "macros.xml");
}

const std::string wxExViMacros::GetRegister(const char name) const
{
  switch (name)
  {
    case '*':
    case '\"': return wxExClipboardGet();
    default: {   
      const auto& it = m_Macros.find(std::string(1, name));
      return it != m_Macros.end() ?
        std::accumulate(it->second.begin(), it->second.end(), std::string()):
        std::string();
    }
  }
}

const std::vector< std::string > wxExViMacros::GetRegisters() const
{
  std::vector< std::string > r;
  
  for (const auto& it : m_Macros)
  {
    if (it.first.size() == 1)
    {
      r.emplace_back(it.first + "=" + wxExSkipWhiteSpace(
        std::accumulate(it.second.begin(), it.second.end(), std::string())));
    }
  }
   
  const std::string clipboard(wxExSkipWhiteSpace(wxExClipboardGet()));
              
  if (!clipboard.empty())
  {
    r.emplace_back("*=" + clipboard);
  }
                
  return r;
}

bool wxExViMacros::IsRecorded(const std::string& macro) const
{
  return !Get(macro).empty();
}

bool wxExViMacros::IsRecordedMacro(const std::string& macro) const
{
  return m_Macros.find(macro) != m_Macros.end();
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
  
  m_Abbreviations.clear();
  m_Macros.clear();
  m_Variables.clear();
  
  wxXmlNode* child = doc.GetRoot()->GetChildren();
  
  while (child)
  {
    if (child->GetName() == "abbreviation")
    {
      ParseNodeAbbreviation(child);
    }
    else if (child->GetName() == "macro")
    {
      ParseNodeMacro(child);
    }
    else if (child->GetName() == "variable")
    {
      ParseNodeVariable(child);
    }
      
    child = child->GetNext();
  }
  
  return true;
}

void wxExViMacros::ParseNodeAbbreviation(wxXmlNode* node)
{
  const std::string abb(node->GetAttribute("name"));
  const std::string text(node->GetNodeContent().Strip(wxString::both));

  if (m_Abbreviations.find(abb) != m_Abbreviations.end())
  {
    wxLogError("Duplicate abbreviation: %s on line: %d", 
     abb,
     node->GetLineNumber());
  }
  else
  {
    m_Abbreviations.insert({abb, text});
  }
}

void wxExViMacros::ParseNodeMacro(wxXmlNode* node)
{
  std::vector<std::string> v;
  
  wxXmlNode* command = node->GetChildren();

  while (command)
  {
    v.emplace_back(Decode(command->GetNodeContent().ToStdString()));
    command = command->GetNext();
  }
  
  const auto& it = m_Macros.find(node->GetAttribute("name").ToStdString());

  if (it != m_Macros.end())
  {
    wxLogError("Duplicate macro: %s on line: %d", 
      node->GetAttribute("name"),
      node->GetLineNumber());
  }
  else
  {
    m_Macros.insert({node->GetAttribute("name").ToStdString(), v});
  }
}

void wxExViMacros::ParseNodeVariable(wxXmlNode* node)
{
  const wxExVariable variable(node);
  const auto& it = m_Variables.find(variable.GetName());

  if (it != m_Variables.end())
  {
    wxLogError("Duplicate variable: %s on line: %d", 
     variable.GetName(),
     node->GetLineNumber());
  }
  else
  {
    m_Variables.insert({variable.GetName(), variable});
  }
}

bool wxExViMacros::Playback(wxExEx* ex, const std::string& macro, int repeat)
{
  if (!IsRecordedMacro(macro))
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
  
  if (!m_IsPlayback && !m_IsRecording)
  {
    m_Macro = macro;
    wxExFrame::StatusText(m_Macro, "PaneMacro");
  }
  
  m_IsPlayback = true;
  
  wxBusyCursor wait;
    
  AskForInput();
  
  const auto& macro_commands(m_Macros[macro]);
  
  for (int i = 0; i < repeat && !stop; i++)
  {
    for (auto& it : macro_commands)
    { 
      stop = !ex->Command(it);
      
      if (stop)
      {
        wxLogStatus(_("Macro aborted at '") + it + "'");
        break;
      }
    }
  }

  ex->GetSTC()->EndUndoAction();

  if (!stop)
  {
    wxLogStatus(_("Macro played back"));
    m_Macro = macro; // might be overridden by expanded variable
    wxExFrame::StatusText(m_Macro, "PaneMacro");
  }
  
  m_IsPlayback = false;
  
  return !stop;
}

void wxExViMacros::Record(const std::string& text, bool new_command)
{
  if (!m_IsRecording || m_IsPlayback || text.empty())
  {
    return;
  }
  
  if (new_command) 
  {
    m_Macros[m_Macro].emplace_back(text);
  }
  else
  {
    if (m_Macros[m_Macro].empty())
    {
      std::string s;
      m_Macros[m_Macro].emplace_back(s);
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
  
  while ((child = root->GetChildren()))
  {
    root->RemoveChild(child);
    delete child;
  }

  std::for_each(m_Macros.rbegin(), m_Macros.rend(), [&](const auto& it)
  {
    if (!it.second.empty())
    {
      wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, "macro");
      element->AddAttribute("name", it.first);
      
      std::for_each(it.second.rbegin(), it.second.rend(), [&](const auto& it2)
      { 
        wxXmlNode* cmd = new wxXmlNode(element, wxXML_ELEMENT_NODE, "command");
        new wxXmlNode(cmd, wxXML_TEXT_NODE, "", Encode(it2));
      });
    }
  });

  std::for_each(m_Variables.rbegin(), m_Variables.rend(), [&](const auto& it)
  {
    wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, "variable");
    it.second.Save(element);
  });
  
  std::for_each(m_Abbreviations.rbegin(), m_Abbreviations.rend(), [&](const auto& it)
  {
    wxXmlNode* element = new wxXmlNode(root, wxXML_ELEMENT_NODE, "abbreviation");
    element->AddAttribute("name", it.first);
    new wxXmlNode(element, wxXML_TEXT_NODE, "", it.second);
  });
  
  const bool ok = doc.Save(GetFileName().GetFullPath());
  
  if (ok)
  {
    m_IsModified = false;
  }

  return ok;
}

void wxExViMacros::SetAbbreviation(const std::string& ab, const std::string& value)
{
  if (value.empty())
  {
    m_Abbreviations.erase(ab);
  }
  else
  {
    m_Abbreviations[ab] = value;
  }
  
  m_IsModified = true;
}

bool wxExViMacros::SetRegister(const char name, const std::string& value)
{
  if (!isalnum(name) && !isdigit(name) && 
       name != '%' && name != '_' && name != '*' && name != '.')
  {
    return false;
  }
  
  if (name == '*')
  {
    wxExClipboardAdd(value);
    return true;
  }

  std::vector<std::string> v;
  
  // The black hole register, everything written to it is discarded.
  if (name != '_')
  {
    if (wxIsupper(name))
    {
      v.emplace_back(GetRegister(tolower(name)) + value);
    }
    else
    {
      v.emplace_back(value);
    }
  }
  
  m_Macros[std::string(1, (char)tolower(name))] = v;
  m_IsModified = true;
  
  return true;
}

void wxExViMacros::StartRecording(const std::string& macro)
{
  if (m_IsRecording || macro.empty())
  {
    return;
  }
  
  m_IsRecording = true;
  m_IsModified = true;
  
  if (macro.size() == 1)
  {
    // We only use lower case macro's, to be able to
    // append to them using.
    m_Macro = macro;
    std::transform(m_Macro.begin(), m_Macro.end(), m_Macro.begin(), ::tolower);
  
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
  
  wxExFrame::StatusText(m_Macro, "PaneMacro");
  
  wxLogStatus(_("Macro recording"));
}

bool wxExViMacros::StartsWith(const std::string& text) const
{
  if (text.empty())
  {
    return false;
  }

  if (wxIsdigit(text[0]))
  {
    return false;
  }
  
  for (const auto& it : m_Macros)
  {
    if (it.first.substr(0, text.size()) == text)
    {
      return true;
    }
  }
   
  for (const auto& it : m_Variables)
  {
    if (it.first.substr(0, text.size()) == text)
    {
      return true;
    }
  }
  
  return false;
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
    m_Macros.erase(m_Macro);
    m_Macro.clear();
    wxLogStatus(wxEmptyString);
    wxExFrame::StatusText(wxEmptyString, "PaneMacro");
  }
}

#endif // wxUSE_GUI
