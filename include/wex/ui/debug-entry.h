////////////////////////////////////////////////////////////////////////////////
// Name:      debug-entry.h
// Purpose:   Declaration of wex::debug_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/core/menu-command.h>
#include <wex/ui/menu-commands.h>

#include <unordered_map>

namespace wex
{
/// This class collects a single debugger.
class debug_entry : public menu_commands<menu_command>
{
public:
  /// The type of regex to match debugger stdout.
  enum class regex_t
  {
    AT_LINE,                 /// at a line number
    AT_PATH_LINE,            /// at a path and a line number
    BREAKPOINT_NO_FILE_LINE, /// a breakpoint no, file and line number
    EXIT,                    /// an 'exit program'
    PATH,                    /// a path
    VARIABLE,                /// a variable e.g. to show in a tooltip
    VARIABLE_MULTI,          /// a variable split over several lines
  };

  /// Default constructor using xml node.
  debug_entry(const pugi::xml_node& node = pugi::xml_node());

  /// Returns the delete breakpoint command.
  const std::string& break_del() const { return m_break_del; }

  /// Returns the set breakpoint command.
  const std::string& break_set() const { return m_break_set; }

  /// Returns the extensions.
  const std::string& extensions() const { return m_extensions; }

  /// Returns the flags.
  const std::string& flags() const { return m_flags; }

  /// Returns the regex for interpreting debug stdout.
  std::string regex_stdout(regex_t r) const;

private:
  std::string m_break_del, m_break_set, m_extensions, m_flags;
  std::unordered_map<regex_t, std::string> m_regex_stdouts;
};
}; // namespace wex
