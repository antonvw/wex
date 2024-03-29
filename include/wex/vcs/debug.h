////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/syntax/marker.h>
#include <wex/ui/debug-entry.h>

#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>

namespace wex
{
namespace factory
{
class process;
}

class frame;
class item_dialog;
class frame;
class menu;
class stc;

/// Offers a debug class that allows you to use an stc component as
/// graphical interface for a debug process (e.g. gdb).
class debug : public wxEvtHandler
{
public:
  // Static interface.

  /// Returns the default debugger for current platform.
  static std::string default_exe();

  // Other methods.

  /// Constructor.
  explicit debug(frame* frame, factory::process* process = nullptr);

  /// Adds debug menu items to specified menu, default as no popup menu.
  /// These menus allow you to interact with the debug process.
  /// Returns number of items added to menu.
  int add_menu(menu* menu, bool popup = false) const;

  /// Applies current breakpoints markers to stc component
  /// if applicable.
  /// Returns false if no markers were added.
  bool apply_breakpoints(stc* stc) const;

  /// Returns breakpoints.
  auto& breakpoints() const { return m_breakpoints; }

  /// Returns current entry.
  const auto& debug_entry() const { return m_entry; }

  /// Executes the item action using the current debug process,
  /// if there is not yet a debug process, invokes frame::process
  /// to allow derived classed to provide one,
  /// and optionally use an stc component for extra input / output.
  /// Returns false if cancelled, or no debug process available.
  bool execute(const std::string& action, stc* stc = nullptr);

  /// As above, but for a menu action item.
  bool execute(int id, stc* stc = nullptr);

  /// Returns true if a debug session is active.
  bool is_active() const;

  /// Ask debugger to print contents of variable.
  /// Returns false if no debug process is available.
  bool print(const std::string& variable) const;

  /// Shows dialog to select debug entry.
  bool show_dialog(wxWindow* parent);

  /// Toggles breakpoint on line.
  /// Returns false if no debug process is available.
  bool toggle_breakpoint(int line, stc* stc);

private:
  bool allow_open(const path& p) const;

  bool clear_breakpoints(const std::string& text);

  std::optional<std::string> get_args(const std::string& command, stc* stc);

  path complete_path(const std::string& text) const;
  void is_finished();
  void process_stdin(const std::string& text);
  void process_stdout(const std::string& text);
  void set_entry(const std::string& debugger);

  const marker m_marker_breakpoint = wex::marker(2);

  /// The breakpoints, relating debugging breakpoint no to
  /// tuple of filename, marker identifier, and line no.
  std::unordered_map<std::string, std::tuple<path, int, int>> m_breakpoints;

  static inline item_dialog* m_dialog = nullptr;

  path m_path, m_path_execution_point;

  frame*                 m_frame;
  wex::debug_entry       m_entry;
  wex::factory::process* m_process{nullptr};
  std::string            m_stdout;
};
}; // namespace wex
