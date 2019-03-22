////////////////////////////////////////////////////////////////////////////////
// Name:      debug_entry.h
// Purpose:   Declaration of wex::debug_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/menucommand.h>
#include <wex/menucommands.h>

namespace wex
{
  class menu;
  
  /// This class collects a single debugger.
  class debug_entry : public menu_commands < menu_command >
  {
  public:
    /// Default constructor.
    debug_entry() {;};
    
    /// Constructor using xml node.
    debug_entry(const pugi::xml_node& node);

    /// Returns the delete breakpoint command.
    const auto& break_del() const {return m_break_del;};

    /// Returns the set breakpoint command.
    const auto& break_set() const {return m_break_set;};

    /// Returns the flags.
    const auto& flags() const {return m_flags;};
  private:
    std::string m_break_set, m_break_del, m_flags;
  };
};
