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

pugi::xml_document wxExViMacros::m_doc;
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
  pugi::xml_node node;
  wxExVariable var;
  const auto& it = m_Variables.find(variable);
    
  if (it == m_Variables.end())
  {
    var = wxExVariable(var);
    m_Variables.insert({variable, var});
    wxLogStatus(_("Added variable") + ": "  +  variable);
    node = m_doc.document_element().append_child("variable");
  }
  else
  {
    try
    {
      const std::string query("//variable[@name='" + variable + "']");
      pugi::xpath_node xp = m_doc.document_element().select_node(query.c_str());

      if (xp && xp.node())
      {
        node = xp.node();
      }
    }
    catch (pugi::xpath_exception& e)
    {
      std::cerr << e.what() << "\n";
    }

    var = it->second;
  }

  const bool ok = var.Expand(ex);

  if (var.IsModified())
  {
    var.Save(node);
    m_IsModified = true;
  }
    
  if (ok)
  {
    wxLogStatus(_("Variable expanded"));
  
    if (!m_IsRecording)
    {
      m_Macro = variable;
    }
  }
  else
  {
    // Now only show log status if this is no input variable,
    // as it was cancelled in that case.    
    if (!it->second.IsInput())
    {
      wxLogStatus(_("Could not expand variable") + ": "  +  variable);
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
      pugi::xml_node node = m_doc.document_element().child("variable");
      pugi::xml_node app = node.append_child(variable.c_str());
      ret.first->second.Save(app);
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
      pugi::xml_node node = m_doc.document_element().child(variable.c_str());
      it->second.Save(node);
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

bool wxExViMacros::LoadDocument()
{
  const pugi::xml_parse_result result = m_doc.load_file(
    GetFileName().GetFullPath().ToStdString().c_str(),
     pugi::parse_default | pugi::parse_comments);

  if (!result)
  {
    wxExXmlError(GetFileName().GetFullPath().ToStdString().c_str(), &result);
    return false;
  }

  m_IsModified = false;
  m_Abbreviations.clear();
  m_Macros.clear();
  m_Variables.clear();
  
  for (const auto& child: m_doc.document_element().children())
  {
    if (strcmp(child.name(), "abbreviation") == 0)
    {
      ParseNodeAbbreviation(child);
    }
    else if (strcmp(child.name(), "macro") == 0)
    {
      ParseNodeMacro(child);
    }
    else if (strcmp(child.name(), "variable") == 0)
    {
      ParseNodeVariable(child);
    }
  }
  
  return true;
}

void wxExViMacros::ParseNodeAbbreviation(const pugi::xml_node& node)
{
  const std::string abb(node.attribute("name").value());

  if (m_Abbreviations.find(abb) != m_Abbreviations.end())
  {
    std::cerr<< "Duplicate abbreviation: " << abb << " with offset: " <<
      node.offset_debug() << "\n";
  }
  else
  {
    m_Abbreviations.insert({abb, std::string(node.text().get())});
  }
}

void wxExViMacros::ParseNodeMacro(const pugi::xml_node& node)
{
  std::vector<std::string> v;
  
  for (const auto& command: node.children())
  {
    v.emplace_back(Decode(command.text().get()));
  }
  
  const auto& it = m_Macros.find(node.attribute("name").value());

  if (it != m_Macros.end())
  {
    std::cerr << "Duplicate macro: " << node.attribute("name").value() << " with offset: " <<
      node.offset_debug() << "\n";
  }
  else
  {
    m_Macros.insert({node.attribute("name").value(), v});
  }
}

void wxExViMacros::ParseNodeVariable(const pugi::xml_node& node)
{
  const wxExVariable variable(node);
  const auto& it = m_Variables.find(variable.GetName());

  if (it != m_Variables.end())
  {
    std::cerr << "Duplicate variable: " << variable.GetName() << " with offset: " <<
      node.offset_debug() << "\n";
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
  
  const bool ok = m_doc.save_file(GetFileName().GetFullPath().ToStdString().c_str());
  
  if (ok)
  {
    m_IsModified = false;
  }

  return ok;
}

void wxExViMacros::SaveMacro(const std::string& macro)
{
  try
  {
    const std::string query("//macro[@name='" + macro + "']");

    pugi::xpath_node node = m_doc.document_element().select_node(query.c_str());

    if (node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    pugi::xml_node node_macro = m_doc.document_element().append_child("macro");
    node_macro.append_attribute("name") = macro.c_str();

    for (const auto& it: m_Macros[macro])
    {
      node_macro.append_child("command").text().set(Encode(it).c_str());
    }
  }
  catch (pugi::xpath_exception& e)
  {
    std::cerr << e.what() << "\n";
  }
}

void wxExViMacros::SetAbbreviation(const std::string& ab, const std::string& value)
{
  try
  {
    const std::string query("//abbreviation[@name='" + ab + "']");
    pugi::xpath_node node = m_doc.document_element().select_node(query.c_str());

    if (node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    if (value.empty())
    {
      m_Abbreviations.erase(ab);
    }
    else
    {
      pugi::xml_node node_ab = m_doc.document_element().append_child("abbreviation");
      node_ab.append_attribute("name") = ab.c_str();
      node_ab.text().set(value.c_str());

      m_Abbreviations[ab] = value;
    }
  }
  catch (pugi::xpath_exception& e)
  {
    std::cerr << e.what() << "\n";
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
  SaveMacro(std::string(1, (char)tolower(name)));

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
  if (text.empty() || wxIsdigit(text[0]))
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
    SaveMacro(m_Macro);
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
