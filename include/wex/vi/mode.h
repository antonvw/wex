////////////////////////////////////////////////////////////////////////////////
// Name:      mode.h
// Purpose:   Declaration of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace wex
{
class vi;
class vi_fsm;

/// Offers vi mode.
/// \dot
/// digraph mode {
///   insert_block [label="insert block"]
///   visual_block [label="visual block"]
///   visual_line  [label="visual line"]
///   init         -> command [style=dotted,label="start"];
///   command      -> insert [label="[acioACIOR]"];
///   command      -> visual [label="v"];
///   command      -> visual_line [label="V"];
///   command      -> visual_block [label="K|control-V"];
///   insert       -> command [label="escape"];
///   insert_block -> visual_block [label="escape"];
///   visual       -> command [label="escape"];
///   visual_block -> command [label="escape"];
///   visual_block -> insert_block [label="[acioACIOR]"];
///   visual_line  -> command [label="escape"];
///  }
/// \enddot
class vi_mode
{
public:
  /// The possible vi mode states.
  enum class state_t
  {
    COMMAND,      ///< command (or navigation) mode
    INSERT,       ///< pressing key inserts key
    INSERT_BLOCK, ///< as insert, while in visual rect mode
    VISUAL,       ///< navigation keys extend selection
    VISUAL_BLOCK, ///< navigation keys extend rectangular selection
    VISUAL_LINE,  ///< complete lines are selected
  };

  /// Constructor,
  vi_mode(
    /// specify vi component
    vi* vi,
    /// method to be called when going into insert mode
    const std::function<void(const std::string& command)>& insert = nullptr,
    /// method to be called when going back to command mode
    const std::function<void()>& f = nullptr);

  /// Destructor.
  ~vi_mode();

  /// Transitions to command mode.
  void command();

  /// Escapes current mode.
  bool escape();

  /// Returns the state we are in.
  state_t get() const;

  /// Returns true if in command mode.
  bool is_command() const { return get() == state_t::COMMAND; }

  /// Returns true if in insert mode.
  bool is_insert() const;

  /// Returns true if in visual mode.
  bool is_visual() const;

  /// Returns insert commands.
  const auto& insert_commands() const { return m_insert_commands; }

  /// Returns mode as a string.
  const std::string str() const;

  /// Transitions to other mode depending on command.
  /// Returns true if command represents a mode change, otherwise false.
  /// If true is returned, it does not mean that mode was changed, in case
  /// of readonly doc.
  bool transition(std::string& command);

  /// Transitions to visual mode.
  void visual();

private:
  bool transition_prep(const std::string& command);

  vi*                                                      m_vi;
  std::unique_ptr<vi_fsm>                                  m_fsm;
  const std::vector<std::pair<int, std::function<void()>>> m_insert_commands;
};
}; // namespace wex
