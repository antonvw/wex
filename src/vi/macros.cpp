////////////////////////////////////////////////////////////////////////////////
// Name:      macros.cpp
// Purpose:   Implementation of class wex::macros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <numeric>
#include <wex/app.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/frame.h>
#include <wex/lexer-props.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/path.h>
#include <wex/type-to-value.h>
#include <wex/util.h>

wex::macros::macros()
  : m_mode(this)
{
}

bool wex::macros::erase()
{
  if (m_macros.erase(m_mode.get_macro()) == 0)
  {
    log("erase macro") << m_mode.get_macro();
    return false;
  }

  return true;
}

const std::vector<std::string> wex::macros::find(const std::string& macro) const
{
  if (const auto& it = m_macros.find(macro); it != m_macros.end())
  {
    return it->second;
  }
  else
  {
    if (const auto& it = m_variables.find(macro); it != m_variables.end())
    {
      return {it->second.get_value()};
    }
    else
    {
      return {};
    }
  }
}

const std::vector<std::string> wex::macros::get() const
{
  std::vector<std::string> v;

  for (const auto& it : m_macros)
  {
    if (it.first.size() > 1)
    {
      v.emplace_back(it.first);
    }
  }

  for (const auto& it : m_variables)
  {
    v.emplace_back(it.first);
  }

  std::sort(v.begin(), v.end());

  return v;
}

const wex::path wex::macros::get_filename() const
{
  return path(config::dir(), "wex-macros.xml");
}

const wex::macros::keys_map_t& wex::macros::get_keys_map(key_t type) const
{
  switch (type)
  {
    case KEY_ALT:
      return m_map_alt_keys;
    case KEY_CONTROL:
      return m_map_control_keys;
    default:
      return m_map_keys;
  }
}

const std::vector<std::string>
wex::macros::get_macro_commands(const std::string& macro) const
{
  const auto& it = m_macros.find(macro);
  return (it != m_macros.end()) ? it->second : std::vector<std::string>();
}

const std::string wex::macros::get_register(char name) const
{
  switch (name)
  {
    case '*':
    case '\"':
      return clipboard_get();

    default:
    {
      const auto& it = m_macros.find(std::string(1, name));
      return it != m_macros.end() ? std::accumulate(
                                      it->second.begin(),
                                      it->second.end(),
                                      std::string()) :
                                    std::string();
    }
  }
}

const std::vector<std::string> wex::macros::get_registers() const
{
  std::vector<std::string> r;
  lexer_props              l;

  for (const auto& it : m_macros)
  {
    if (it.first.size() == 1)
    {
      r.emplace_back(l.make_key(
        it.first,
        boost::algorithm::trim_copy(
          std::accumulate(it.second.begin(), it.second.end(), std::string()))));
    }
  }

  if (const auto& clipboard(boost::algorithm::trim_copy(clipboard_get()));
      !clipboard.empty())
  {
    r.emplace_back(l.make_key("*", clipboard));
  }

  return r;
}

bool wex::macros::is_recorded(const std::string& macro) const
{
  return !find(macro).empty();
}

bool wex::macros::is_recorded_macro(const std::string& macro) const
{
  return m_macros.find(macro) != m_macros.end();
}

bool wex::macros::load_document()
{
  if (!get_filename().file_exists())
  {
    return false;
  }

  if (const auto result = m_doc.load_file(
        get_filename().string().c_str(),
        pugi::parse_default | pugi::parse_comments);
      !result)
  {
    xml_error(get_filename(), &result);
    return false;
  }

  m_is_modified = false;

  if (m_is_loaded)
  {
    m_abbreviations.clear();
    m_macros.clear();
    m_map.clear();
    m_map_alt_keys.clear();
    m_map_control_keys.clear();
    m_map_keys.clear();
    m_variables.clear();
  }

  for (const auto& child : m_doc.document_element().children())
  {
    if (strcmp(child.name(), "abbreviation") == 0)
    {
      parse_node<std::string, strings_map_t>(
        child,
        "abbreviation",
        m_abbreviations);
    }
    else if (strcmp(child.name(), "macro") == 0)
    {
      parse_node_macro(child);
    }
    else if (strcmp(child.name(), "map") == 0)
    {
      parse_node<std::string, strings_map_t>(child, "map", m_map);
    }
    else if (strcmp(child.name(), "map-alt") == 0)
    {
      parse_node<int, keys_map_t>(child, "map-alt", m_map_alt_keys);
    }
    else if (strcmp(child.name(), "map-control") == 0)
    {
      parse_node<int, keys_map_t>(child, "map-control", m_map_control_keys);
    }
    else if (strcmp(child.name(), "map-key") == 0)
    {
      parse_node<int, keys_map_t>(child, "map-key", m_map_keys);
    }
    else if (strcmp(child.name(), "variable") == 0)
    {
      parse_node_variable(child);
    }
  }

  log::trace("macros info")
    << "abbreviations:" << m_abbreviations.size() << "maps:" << m_map.size()
    << "macros:" << m_macros.size() << "variables:" << m_variables.size();

  m_is_loaded = true;

  return true;
}

template <typename S, typename T>
void wex::macros::parse_node(
  const pugi::xml_node& node,
  const std::string&    container_name,
  T&                    container)
{
  const std::string name(node.attribute("name").value());

  if (const S& value = type_to_value<S>(name).get();
      container.find(value) != container.end())
  {
    log("duplicate " + container_name) << name << "current:" << container[value]
                                       << "upate:" << node.text().get() << node;
  }
  else
  {
    container.insert({value, std::string(node.text().get())});
  }
}

void wex::macros::parse_node_macro(const pugi::xml_node& node)
{
  const std::string name(node.attribute("name").value());

  if (const auto& it = m_macros.find(name); it != m_macros.end())
  {
    log("duplicate macro") << name << node;
  }
  else
  {
    std::vector<std::string> v;

    std::transform(
      node.children().begin(),
      node.children().end(),
      std::back_inserter(v),
      [](const auto& t) {
        return t.text().get();
      });

    m_macros.insert({node.attribute("name").value(), v});
  }
}

void wex::macros::parse_node_variable(const pugi::xml_node& node)
{
  if (const variable & variable(node); variable.get_name().empty())
  {
    log("empty variable") << node;
  }
  else if (const auto& it = m_variables.find(variable.get_name());
           it != m_variables.end())
  {
    log("duplicate variable") << variable.get_name() << node;
  }
  else
  {
    m_variables.insert({variable.get_name(), variable});
  }
}

bool wex::macros::record(const std::string& text, bool new_command)
{
  if (auto* f = (dynamic_cast<frame*>(wxTheApp->GetTopWindow())); f != nullptr)
  {
    f->record(text);
  }

  if (!m_mode.is_recording() || m_mode.is_playback() || text.empty())
  {
    return false;
  }

  if (m_mode.get_macro().empty())
  {
    log("macro record while macro empty") << text;
    return false;
  }

  if (new_command)
  {
    m_macros[m_mode.get_macro()].emplace_back(text == " " ? "l" : text);
  }
  else
  {
    if (m_macros[m_mode.get_macro()].empty())
    {
      std::string s;
      m_macros[m_mode.get_macro()].emplace_back(s);
    }

    m_macros[m_mode.get_macro()].back() += text;
  }

  if (const auto& it = m_macros.find(m_mode.get_macro()); it != m_macros.end())
  {
    log::trace("macro recorded")
      << m_mode.get_macro() << "->"
      << std::accumulate(it->second.begin(), it->second.end(), std::string());
  }

  return true;
}

bool wex::macros::save_document(bool only_if_modified)
{
  if (
    !m_is_loaded || !get_filename().file_exists() ||
    (!m_is_modified && only_if_modified))
  {
    return false;
  }

  const bool ok = m_doc.save_file(get_filename().string().c_str(), "  ");

  if (ok)
  {
    m_is_modified = false;
  }

  return ok;
}

void wex::macros::save_macro(const std::string& macro)
{
  try
  {
    if (const auto& node = m_doc.document_element().select_node(
          std::string("//macro[@name='" + macro + "']").c_str());
        node && node.node())
    {
      m_doc.document_element().remove_child(node.node());
    }

    auto node_macro = m_doc.document_element().append_child("macro");
    node_macro.append_attribute("name") = macro.c_str();

    for (const auto& it : m_macros[macro])
    {
      node_macro.append_child("command").text().set(it.c_str());
    }

    m_is_modified = true;

    save_document();
  }
  catch (pugi::xpath_exception& e)
  {
    log(e) << macro;
  }
}

template <typename S, typename T>
void wex::macros::set(
  T&                 container,
  const std::string& xpath,
  const std::string& name,
  const std::string& value)
{
  try
  {
    if (const auto& node = m_doc.document_element().select_node(
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
      auto child = m_doc.document_element().append_child(xpath.c_str());
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

void wex::macros::set_abbreviation(
  const std::string& name,
  const std::string& value)
{
  set<std::string, strings_map_t>(m_abbreviations, "abbreviation", name, value);
}

void wex::macros::set_key_map(
  const std::string& name,
  const std::string& value,
  macros::key_t      type)
{
  switch (type)
  {
    case KEY_ALT:
      set<int, keys_map_t>(m_map_alt_keys, "map-alt", name, value);
      break;

    case KEY_CONTROL:
      set<int, keys_map_t>(m_map_control_keys, "map-control", name, value);
      break;

    case KEY_NORMAL:
      set<int, keys_map_t>(m_map_keys, "map-key", name, value);
      break;
  }
}

void wex::macros::set_map(const std::string& name, const std::string& value)
{
  set<std::string, strings_map_t>(m_map, "map", name, value);
}

bool wex::macros::set_register(char name, const std::string& value)
{
  if (
    !isalnum(name) && !isdigit(name) && name != '%' && name != '_' &&
    name != '*' && name != '.')
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

  m_macros[std::string(1, (char)tolower(name))] = v;
  save_macro(std::string(1, (char)tolower(name)));

  return true;
}

bool wex::macros::starts_with(const std::string_view& text)
{
  if (text.empty() || isdigit(text[0]))
  {
    return false;
  }

  return (
    std::any_of(
      m_macros.begin(),
      m_macros.end(),
      [text](const auto& p) {
        return p.first.substr(0, text.size()) == text;
      }) ||
    std::any_of(m_variables.begin(), m_variables.end(), [text](const auto& p) {
      return p.first.substr(0, text.size()) == text;
    }));
}
