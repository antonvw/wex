////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wex::debug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <tuple>
#include <wx/extension/marker.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>
#include <wx/extension/path.h>

namespace wex
{
  class item_dialog;
  class managed_frame;
  class menu;
  class process;
  class stc;

  /// Offers a debug that allows you to use an stc component as
  /// graphical interface for a debug process (e.g. gdb).
  class debug
  {
  public:
    /// Constructor.
    debug(managed_frame* frame, process* process = nullptr);

    /// Adds debug menu items to specified menu, default as no popup menu.
    /// These menus allow you to interact with the debug process.
    /// Returns number of items added to menu.
    int AddMenu(menu* menu, bool popup = false) const;
    
    /// Executes the item action using the current debug process,
    /// if there is not yet a debug process, invokes frame::Process
    /// to allow derived classed to provide one,
    /// and optionally use an stc component for extra input / output.
    /// Returns false if cancelled, or no debug process available.
    bool Execute(const std::string& action, stc* stc = nullptr);
    
    /// As above, but for a menu action item.
    bool Execute(int item, stc* stc = nullptr) {
      return 
        item < (int)m_Entry.GetCommands().size() &&
        Execute(m_Entry.GetCommands().at(item).GetCommand(), stc);};

    /// Returns brekpoints.
    auto & GetBreakpoints() {return m_Breakpoints;};
    
    /// Returns marker for brekpoint.
    const auto & GetMarkerBreakpoint() const {return m_MarkerBreakpoint;};

    /// Returns process.
    auto GetProcess() {return m_Process;};

    /// Handles stdin from process.
    void ProcessStdIn(const std::string& text);
    
    /// Handles stdout from process.
    void ProcessStdOut(const std::string& text);
  private:
    bool DeleteAllBreakpoints(const std::string& text);
    bool GetArgs(
      const std::string& command, std::string& args, stc* stc);

    /// Marker for a breakpoint.
    const marker m_MarkerBreakpoint = wex::marker(2);
    
    /// The breakpoints, relating debugging breakpoint no to
    /// tuple of filename, marker identifier, and line no.
    std::map<
      std::string, std::tuple<path, int, int>> m_Breakpoints;

    static inline item_dialog* m_Dialog = nullptr;  
    path m_Path;
    managed_frame* m_Frame;
    menu_commands< menu_command> m_Entry;
    process* m_Process {nullptr};
  };
};
