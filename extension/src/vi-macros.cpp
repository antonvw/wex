////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros.cpp
// Purpose:   Implementation of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-macros.h>
#include <wx/extension/log.h>
#include <wx/extension/path.h>
#include <wx/extension/type-to-value.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros-mode.h>

pugi::xml_document wxExViMacros::m_doc;
bool wxExViMacros::m_IsModified = false;
std::string wxExViMacros::m_Macro;
wxExViMacrosMode* wxExViMacros::m_Mode = nullptr;

std::map <std::string, std::string> wxExViMacros::m_Abbreviations;
std::map <std::string, std::vector< std::string > > wxExViMacros::m_Macros;
std::map<std::string, std::string> wxExViMacros::m_Map;
wxExViMacrosMapType wxExViMacros::m_MapAltKeys;
wxExViMacrosMapType wxExViMacros::m_MapControlKeys;
wxExViMacrosMapType wxExViMacros::m_MapKeys;
std::map <std::string, wxExVariable > wxExViMacros::m_Variables;

wxExViMacros::wxExViMacros()
{ 
  if (m_Mode == nullptr)
  {
    m_Mode = new wxExViMacrosMode();
  }
}

const std::vector< std::string > wxExViMacros::Get()
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

const std::vector< std::string > wxExViMacros::Get(const std::string& macro)
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

const wxExPath wxExViMacros::GetFileName()
{
  return wxExPath(wxExConfigDir(), "macros.xml");
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

bool wxExViMacros::IsRecorded(const std::string& macro)
{
  return !Get(macro).empty();
}

bool wxExViMacros::IsRecordedMacro(const std::string& macro)
{
  return m_Macros.find(macro) != m_Macros.end();
}

bool wxExViMacros::LoadDocument()
{
  if (!GetFileName().FileExists())
  {
    return false;
  }

  const pugi::xml_parse_result result = m_doc.load_file(
    GetFileName().Path().string().c_str(),
    pugi::parse_default | pugi::parse_comments);

  if (!result)
  {
    wxExXmlError(GetFileName(), &result);
    return false;
  }

  if (m_Mode == nullptr)
  {
    m_Mode = new wxExViMacrosMode();
  }

  m_IsModified = false;
  m_Abbreviations.clear();
  m_Macros.clear();
  m_Map.clear();
  m_MapAltKeys.clear();
  m_MapControlKeys.clear();
  m_MapKeys.clear();
  m_Variables.clear();
  
  for (const auto& child: m_doc.document_element().children())
  {
    if (strcmp(child.name(), "abbreviation") == 0)
    {
      ParseNode<std::string, std::map <std::string, std::string>> (
        child, "abbreviation", m_Abbreviations);
    }
    else if (strcmp(child.name(), "macro") == 0)
    {
      ParseNodeMacro(child);
    }
    else if (strcmp(child.name(), "map") == 0)
    {
      ParseNode<std::string, std::map<std::string, std::string>>(child, "map", m_Map);
    }
    else if (strcmp(child.name(), "map-alt") == 0)
    {
      ParseNode<int, wxExViMacrosMapType>(child, "map-alt", m_MapAltKeys);
    }
    else if (strcmp(child.name(), "map-control") == 0)
    {
      ParseNode<int, wxExViMacrosMapType>(child, "map-control", m_MapControlKeys);
    }
    else if (strcmp(child.name(), "map-key") == 0)
    {
      ParseNode<int, wxExViMacrosMapType>(child, "map-key", m_MapKeys);
    }
    else if (strcmp(child.name(), "variable") == 0)
    {
      ParseNodeVariable(child);
    }
  }
  
  return true;
}

template <typename S, typename T> 
void wxExViMacros::ParseNode(
  const pugi::xml_node& node,
  const std::string& name,
  T & container)
{
  const S value = wxExTypeToValue<S>(node.attribute("name").value()).get();

  if (container.find(value) != container.end())
  {
    wxExLog() << "duplicate " << 
      name << ": " << value << " from: " << 
      node.attribute("name").value() << node;
  }
  else
  {
    container.insert({value, std::string(node.text().get())});
  }
}

void wxExViMacros::ParseNodeMacro(const pugi::xml_node& node)
{
  std::vector<std::string> v;
  
  for (const auto& command: node.children())
  {
    v.emplace_back(command.text().get());
  }
  
  const auto& it = m_Macros.find(node.attribute("name").value());

  if (it != m_Macros.end())
  {
    wxExLog() << "duplicate macro: " << node.attribute("name").value() << node;
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
    wxExLog() << "duplicate variable: " << variable.GetName() << node;
  }
  else
  {
    m_Variables.insert({variable.GetName(), variable});
  }
}

void wxExViMacros::Record(const std::string& text, bool new_command)
{
  if (!m_Mode->IsRecording() || text.empty())
  {
    return;
  }
  
  if (new_command) 
  {
    // TODO: Improve.
    m_Macros[m_Macro].emplace_back(text == " " ? "l": text);
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
  if (!GetFileName().FileExists() || (!m_IsModified && only_if_modified))
  {
    return false;
  }
  
  const bool ok = m_doc.save_file(GetFileName().Path().string().c_str());
  
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
      node_macro.append_child("command").text().set(it.c_str());
    }

    m_IsModified = true;
  }
  catch (pugi::xpath_exception& e)
  {
    wxExLog(e) << macro;
  }
}

template <typename S, typename T> 
void wxExViMacros::Set(
  T  & container,
  const std::string& xpath,
  const std::string& name,
  const std::string& value)
{
  try
  {
    const std::string query("//" + xpath + "[@name='" + name + "']");
    pugi::xpath_node node = m_doc.document_element().select_node(query.c_str());

    if (node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    if (value.empty())
    {
      container.erase(wxExTypeToValue<S>(name).get());
    }
    else
    {
      pugi::xml_node child = m_doc.document_element().append_child(xpath.c_str());
      child.append_attribute("name") = name.c_str();
      child.text().set(value.c_str());

      container[wxExTypeToValue<S>(name).get()] = value;
    }

    m_IsModified = true;
  }
  catch (pugi::xpath_exception& e)
  {
    wxExLog(e) << name;
  }
}

void wxExViMacros::SetAbbreviation(const std::string& name, const std::string& value)
{
  Set<std::string, std::map<std::string, std::string>>(m_Abbreviations, "abbreviation", name, value);
}

void wxExViMacros::SetKeyMap(
  const std::string& name, 
  const std::string& value,
  wxExViMacrosKeyType type)
{
  switch (type)
  {
    case KEY_ALT: Set<int, wxExViMacrosMapType>(m_MapAltKeys, "map-alt", name, value); break;
    case KEY_CONTROL: Set<int, wxExViMacrosMapType>(m_MapControlKeys, "map-control", name, value); break;
    case KEY_NORMAL: Set<int, wxExViMacrosMapType>(m_MapKeys, "map-key", name, value); break;
  }
}

void wxExViMacros::SetMap(
  const std::string& name, 
  const std::string& value)
{
  Set<std::string, std::map<std::string, std::string>>(m_Map, "map", name, value);
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
    if (isupper(name))
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

  return true;
}

bool wxExViMacros::StartsWith(const std::string_view& text)
{
  if (text.empty() || isdigit(text[0]))
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
