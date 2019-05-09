////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <wx/event.h>
#include <wex/debug-entry.h>
#include <wex/marker.h>
#include <wex/path.h>

namespace wex
{
  class item_dialog;
  class managed_frame;
  class menu;
  class process;
  class stc;

  /// Offers a debug that allows you to use an stc component as
  /// graphical interface for a debug process (e.g. gdb).
  class debug : public wxEvtHandler
  {
  public:
    /// Constructor.
    debug(managed_frame* frame, process* process = nullptr);

    /// Adds debug menu items to specified menu, default as no popup menu.
    /// These menus allow you to interact with the debug process.
    /// Returns number of items added to menu.
    int add_menu(menu* menu, bool popup = false) const;
    
    /// Returns brekpoints.
    auto & breakpoints() {return m_Breakpoints;};
    
    /// Returns current entry.
    const auto & debug_entry() const {return m_Entry;};
    
    /// Executes the item action using the current debug process,
    /// if there is not yet a debug process, invokes frame::Process
    /// to allow derived classed to provide one,
    /// and optionally use an stc component for extra input / output.
    /// Returns false if cancelled, or no debug process available.
    bool execute(const std::string& action, stc* stc = nullptr);
    
    /// As above, but for a menu action item.
    bool execute(int item, stc* stc = nullptr) {
      return 
        item < (int)m_Entry.get_commands().size() &&
        execute(m_Entry.get_commands().at(item).get_command(), stc);};

    /// Returns marker for brekpoint.
    const auto & marker_breakpoint() const {return m_MarkerBreakpoint;};
    
    /// Prints contens of variable.
    const std::string print(const std::string& variable);
    
    /// Returns process.
    auto * process() {return m_Process;};

    /// Shows dialog to select debug entry.
    bool show_dialog(frame* parent);

    /// Toggles breakpoint on line.
    void toggle_breakpoint(int line, stc* stc);
  private:
    bool clear_breakpoints(const std::string& text);
    std::tuple<bool, std::string> get_args(
      const std::string& command, stc* stc);
    /// Handles stdin from process.
    void process_stdin(const std::string& text);
    /// Handles stdout from process.
    void process_stdout(const std::string& text);
    bool set_entry(const std::string& debugger);

    /// Marker for a breakpoint.
    const marker m_MarkerBreakpoint = wex::marker(2);
    
    /// The breakpoints, relating debugging breakpoint no to
    /// tuple of filename, marker identifier, and line no.
    std::map<
      std::string, std::tuple<path, int, int>> m_Breakpoints;

    static inline item_dialog* m_Dialog = nullptr;  
    path m_Path;
    managed_frame* m_Frame;
    wex::debug_entry m_Entry;
    wex::process* m_Process {nullptr};
  };
};
