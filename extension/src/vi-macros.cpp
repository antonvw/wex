////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros.cpp
// Purpose:   Implementation of class wex::vi_macros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <wex/vi-macros.h>
#include <wex/config.h>
#include <wex/lexer-props.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/type-to-value.h>
#include <wex/util.h>
#include <wex/vi-macros-mode.h>

pugi::xml_document wex::vi_macros::m_doc;
bool wex::vi_macros::m_is_modified = false;
std::string wex::vi_macros::m_Macro;
wex::vi_macros_mode* wex::vi_macros::m_Mode = nullptr;

std::map <std::string, std::string> wex::vi_macros::m_Abbreviations;
std::map <std::string, std::vector< std::string > > wex::vi_macros::m_Macros;
std::map<std::string, std::string> wex::vi_macros::m_Map;
wex::vi_macros_maptype wex::vi_macros::m_MapAltKeys;
wex::vi_macros_maptype wex::vi_macros::m_MapControlKeys;
wex::vi_macros_maptype wex::vi_macros::m_MapKeys;
std::map <std::string, wex::variable > wex::vi_macros::m_Variables;

wex::vi_macros::vi_macros()
{ 
  if (m_Mode == nullptr)
  {
    m_Mode = new vi_macros_mode();
  }
}

const std::vector< std::string > wex::vi_macros::get()
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

const std::vector< std::string > wex::vi_macros::get(const std::string& macro)
{
  if (const auto& it = m_Macros.find(macro); it != m_Macros.end())
  {
    return it->second;
  }
  else
  {
    if (const auto& it = m_Variables.find(macro); it != m_Variables.end())
    {
      return {it->second.get_value()};
    }
    else 
    {
      return {};
    }
  }
}

const wex::path wex::vi_macros::get_filename()
{
  return path(config().dir(), "macros.xml");
}

const std::string wex::vi_macros::get_register(const char name) const
{
  switch (name)
  {
    case '*':
    case '\"': return clipboard_get();
    default: {   
      const auto& it = m_Macros.find(std::string(1, name));
      return it != m_Macros.end() ?
        std::accumulate(it->second.begin(), it->second.end(), std::string()):
        std::string();
    }
  }
}

const std::vector< std::string > wex::vi_macros::registers() const
{
  std::vector< std::string > r;
  lexer_props l;
  
  for (const auto& it : m_Macros)
  {
    if (it.first.size() == 1)
    {
      r.emplace_back(l.make_key(it.first, skip_white_space(
        std::accumulate(it.second.begin(), it.second.end(), std::string()))));
    }
  }
   
  if (const std::string clipboard(skip_white_space(clipboard_get())); !clipboard.empty())
  {
    r.emplace_back(l.make_key("*", clipboard));
  }
                
  return r;
}

bool wex::vi_macros::is_recorded(const std::string& macro)
{
  return !get(macro).empty();
}

bool wex::vi_macros::is_recorded_macro(const std::string& macro)
{
  return m_Macros.find(macro) != m_Macros.end();
}

bool wex::vi_macros::load_document()
{
  if (!get_filename().file_exists())
  {
    return false;
  }

  if (const auto result = m_doc.load_file(get_filename().data().string().c_str(),
    pugi::parse_default | pugi::parse_comments);
    !result)
  {
    xml_error(get_filename(), &result);
    return false;
  }

  if (m_Mode == nullptr)
  {
    m_Mode = new vi_macros_mode();
  }

  m_is_modified = false;
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
      ParseNode<int, vi_macros_maptype>(child, "map-alt", m_MapAltKeys);
    }
    else if (strcmp(child.name(), "map-control") == 0)
    {
      ParseNode<int, vi_macros_maptype>(child, "map-control", m_MapControlKeys);
    }
    else if (strcmp(child.name(), "map-key") == 0)
    {
      ParseNode<int, vi_macros_maptype>(child, "map-key", m_MapKeys);
    }
    else if (strcmp(child.name(), "variable") == 0)
    {
      ParseNodeVariable(child);
    }
  }
  
  return true;
}

template <typename S, typename T> 
void wex::vi_macros::ParseNode(
  const pugi::xml_node& node,
  const std::string& name,
  T & container)
{
  if (const S value = type_to_value<S>(node.attribute("name").value()).get();
    container.find(value) != container.end())
  {
    log() << "duplicate" << 
      name << ":" << value << "from:" << 
      node.attribute("name").value() << node;
  }
  else
  {
    container.insert({value, std::string(node.text().get())});
  }
}

void wex::vi_macros::ParseNodeMacro(const pugi::xml_node& node)
{
  std::vector<std::string> v;
  
  for (const auto& command: node.children())
  {
    v.emplace_back(command.text().get());
  }
  
  if (const auto& it = m_Macros.find(node.attribute("name").value());
    it != m_Macros.end())
  {
    log() << "duplicate macro:" << node.attribute("name").value() << node;
  }
  else
  {
    m_Macros.insert({node.attribute("name").value(), v});
  }
}

void wex::vi_macros::ParseNodeVariable(const pugi::xml_node& node)
{
  const variable variable(node);

  if (variable.get_name().empty())
  {
    log() << "empty variable:" << node;
    return;
  }

  if (const auto& it = m_Variables.find(variable.get_name());
    it != m_Variables.end())
  {
    log() << "duplicate variable:" << variable.get_name() << node;
  }
  else
  {
    m_Variables.insert({variable.get_name(), variable});
  }
}

void wex::vi_macros::record(const std::string& text, bool new_command)
{
  if (!m_Mode->is_recording() || text.empty())
  {
    return;
  }
  
  if (new_command) 
  {
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

bool wex::vi_macros::save_document(bool only_if_modified)
{
  if (!get_filename().file_exists() || (!m_is_modified && only_if_modified))
  {
    return false;
  }
  
  const bool ok = m_doc.save_file(get_filename().data().string().c_str());
  
  if (ok)
  {
    m_is_modified = false;
  }

  return ok;
}

void wex::vi_macros::save_macro(const std::string& macro)
{
  try
  {
    if (auto node = m_doc.document_element().select_node(
      std::string("//macro[@name='" + macro + "']").c_str());
      node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    pugi::xml_node node_macro = m_doc.document_element().append_child("macro");
    node_macro.append_attribute("name") = macro.c_str();

    for (const auto& it: m_Macros[macro])
    {
      node_macro.append_child("command").text().set(it.c_str());
    }

    m_is_modified = true;
  }
  catch (pugi::xpath_exception& e)
  {
    log(e) << macro;
  }
}

template <typename S, typename T> 
void wex::vi_macros::Set(
  T  & container,
  const std::string& xpath,
  const std::string& name,
  const std::string& value)
{
  try
  {
    if (auto node = m_doc.document_element().select_node(
      std::string("//" + xpath + "[@name='" + name + "']").c_str());
      node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    if (value.empty())
    {
      container.erase(type_to_value<S>(name).get());
    }
    else
    {
      pugi::xml_node child = m_doc.document_element().append_child(xpath.c_str());
      child.append_attribute("name") = name.c_str();
      child.text().set(value.c_str());

      container[type_to_value<S>(name).get()] = value;
    }

    m_is_modified = true;
  }
  catch (pugi::xpath_exception& e)
  {
    log(e) << name;
  }
}

void wex::vi_macros::set_abbreviation(const std::string& name, const std::string& value)
{
  Set<std::string, std::map<std::string, std::string>>(m_Abbreviations, "abbreviation", name, value);
}

void wex::vi_macros::set_key_map(
  const std::string& name, 
  const std::string& value,
  vi_macros::key_t type)
{
  switch (type)
  {
    case KEY_ALT: Set<int, vi_macros_maptype>(m_MapAltKeys, "map-alt", name, value); break;
    case KEY_CONTROL: Set<int, vi_macros_maptype>(m_MapControlKeys, "map-control", name, value); break;
    case KEY_NORMAL: Set<int, vi_macros_maptype>(m_MapKeys, "map-key", name, value); break;
  }
}

void wex::vi_macros::set_map(
  const std::string& name, 
  const std::string& value)
{
  Set<std::string, std::map<std::string, std::string>>(m_Map, "map", name, value);
}

bool wex::vi_macros::set_register(const char name, const std::string& value)
{
  if (!isalnum(name) && !isdigit(name) && 
       name != '%' && name != '_' && name != '*' && name != '.')
  {
    return false;
  }
  
  if (name == '*')
  {
    clipboard_add(value);
    return true;
  }

  std::vector<std::string> v;
  
  // The black hole register, everything written to it is discarded.
  if (name != '_')
  {
    if (isupper(name))
    {
      v.emplace_back(get_register(tolower(name)) + value);
    }
    else
    {
      v.emplace_back(value);
    }
  }
  
  m_Macros[std::string(1, (char)tolower(name))] = v;
  save_macro(std::string(1, (char)tolower(name)));

  return true;
}

bool wex::vi_macros::starts_with(const std::string_view& text)
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
