////////////////////////////////////////////////////////////////////////////////
// Name:      debug_entry.cpp
// Purpose:   Implementation of wex::debug_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/debug-entry.h>

wex::debug_entry::debug_entry(const pugi::xml_node& node)
  : menu_commands(node)
  , m_flags(node.attribute("flags").value())
  , m_break_del(node.attribute("break-del").value())
  , m_break_set(node.attribute("break-set").value())
{
}
