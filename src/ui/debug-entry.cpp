////////////////////////////////////////////////////////////////////////////////
// Name:      debug_entry.cpp
// Purpose:   Implementation of wex::debug_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/debug-entry.h>

wex::debug_entry::debug_entry(const pugi::xml_node& node)
  : menu_commands(node)
  , m_flags(node.attribute("flags").value())
  , m_break_del(node.attribute("break-del").value())
  , m_break_set(node.attribute("break-set").value())
  , m_extensions(node.attribute("extensions").value())
  , m_regex_stdouts(
      {{regex_t::AT_LINE, node.attribute("regex-at-line").value()},
       {regex_t::AT_PATH_LINE, node.attribute("regex-at-path-line").value()},
       {regex_t::EXIT, node.attribute("regex-exit").value()},
       {regex_t::BREAKPOINT_NO_FILE_LINE,
        node.attribute("regex-no-file-line").value()},
       {regex_t::PATH, node.attribute("regex-path").value()},
       {regex_t::VARIABLE, node.attribute("regex-variable").value()},
       {regex_t::VARIABLE_MULTI, node.attribute("regex-variable-m").value()}})
{
}

std::string wex::debug_entry::regex_stdout(regex_t r) const
{
  const auto& i = m_regex_stdouts.find(r);
  return i != m_regex_stdouts.end() ? i->second : std::string();
}
