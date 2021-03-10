////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <wex/debug-entry.h>
#include <wex/marker.h>
#include <wex/path.h>

namespace wex
{
  namespace core
  {
    class process;
  }
  class frame;
  class item_dialog;
  class managed_frame;
  class menu;
  class stc;

  /// Offers a debug that allows you to use an stc component as
  /// graphical interface for a debug process (e.g. gdb).
  class debug : public wxEvtHandler
  {
  public:
    /// Constructor.
    debug(managed_frame* frame, core::process* process = nullptr);

    /// Adds debug menu items to specified menu, default as no popup menu.
    /// These menus allow you to interact with the debug process.
    /// Returns number of items added to menu.
    int add_menu(menu* menu, bool popup = false) const;

    /// Applies current breakpoints markers to stc component
    /// if applicable.
    /// Returns false if no markers were added.
    bool apply_breakpoints(stc* stc) const;

    /// Returns breakpoints.
    auto& breakpoints() const { return m_breakpoints; };

    /// Returns current entry.
    const auto& debug_entry() const { return m_entry; };

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
    bool show_dialog(frame* parent);

    /// Toggles breakpoint on line.
    /// Returns false if no debug process is available.
    bool toggle_breakpoint(int line, stc* stc);

  private:
    bool allow_open(const path& p) const;

    bool clear_breakpoints(const std::string& text);

    std::tuple<bool, std::string>
    get_args(const std::string& command, stc* stc);

    void is_finished();

    void set_entry(const std::string& debugger);

    const marker m_marker_breakpoint = wex::marker(2);

    /// The breakpoints, relating debugging breakpoint no to
    /// tuple of filename, marker identifier, and line no.
    std::map<std::string, std::tuple<path, int, int>> m_breakpoints;

    static inline item_dialog* m_dialog = nullptr;

    path m_path, m_path_execution_point;

    managed_frame*   m_frame;
    wex::debug_entry m_entry;
    wex::core::process* m_process{nullptr};
    bool             m_active{false};
    std::string      m_stdout;
  };
}; // namespace wex
