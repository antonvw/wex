////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-command.h
// Purpose:   Declaration of wex::vcs_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/menu-command.h>

namespace wex
{
/// This class contains a single vcs command.
class vcs_command : public menu_command
{
public:
  /// Default constructor using xml node.
  vcs_command(const pugi::xml_node& node = pugi::xml_node())
    : menu_command(node)
  {
    ;
  };

  /// Returns true if this is a add like command.
  bool is_add() const;

  /// Returns true if this is a blame like command.
  bool is_blame() const;

  /// Returns true if this is a checkout like command.
  bool is_checkout() const;

  /// Returns true if this is a commit like command.
  bool is_commit() const;

  /// Returns true if this is a diff like command.
  bool is_diff() const;

  /// Returns true if this is a history like command.
  bool is_history() const;

  /// Returns true if this command can behave like
  /// opening a file.
  bool is_open() const;
};
}; // namespace wex
