////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wxExDebug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <wx/extension/filename.h>
#include <wx/extension/marker.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>

class wxExManagedFrame;
class wxExMenu;
class wxExProcess;
class wxExSTC;

/// Offers a wxExDebug that allows you to use an wxExSTC component as
/// graphical interface for a debug process (e.g. gdb).
class WXDLLIMPEXP_BASE wxExDebug
{
public:
  /// Constructor.
  wxExDebug(wxExManagedFrame* frame, wxExProcess* process = nullptr);

  /// Adds debug menu items to specified menu, default as no popup menu.
  /// These menus allow you to interact with the debug process.
  /// Returns number of items added to menu.
  int AddMenu(wxExMenu* menu, bool popup = false) const;
  
  /// Executes the menu item action using the current debug process,
  /// if there is not yet a debug process, invokes wxExFrame::Process
  /// to allow derived classed to provide one,
  /// and optionally use an stc component for extra input / output.
  /// If the action is not part of menu commands, false is returned.
  bool Execute(const std::string& item, wxExSTC* stc = nullptr);
  
  /// As above, but for a menu action item.
  bool Execute(int item, wxExSTC* stc = nullptr) {
    return 
      item < m_Entry.GetCommands().size() &&
      Execute(m_Entry.GetCommands().at(item).GetCommand(), stc);};

  /// Returns brekpoints.
  const auto & GetBreakpoints() const {return m_Breakpoints;};
  
  /// Returns marker for brekpoint.
  const auto & GetMarkerBreakpoint() const {return m_MarkerBreakpoint;};

  /// Returns process.
  auto GetProcess() {return m_Process;};

  /// Handles input from process.
  void ProcessInput(const std::string& text);
  
  /// Handles output from process.
  void ProcessOutput(const std::string& text);
private:
  bool GetArgs(
    const std::string& command, std::string& args, wxExSTC* stc);

  /// Marker for a breakpoint.
  const wxExMarker m_MarkerBreakpoint = wxExMarker(2);
  
  /// The breakpoints, relating debugging breakpoint no to
  /// tuple of filename, marker identifier, and line no.
  std::map<
    std::string, std::tuple<wxExFileName, int, int>> m_Breakpoints;
  
  wxExFileName m_FileName;
  wxExManagedFrame* m_Frame;
  wxExMenuCommands< wxExMenuCommand> m_Entry;
  wxExProcess* m_Process = nullptr;
};
