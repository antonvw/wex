////////////////////////////////////////////////////////////////////////////////
// Name:      debug.h
// Purpose:   Declaration of class wxExDebug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <wx/extension/marker.h>
#include <wx/extension/menucommand.h>
#include <wx/extension/menucommands.h>

class wxExManagedFrame;
class wxExMenu;
class wxExProcess;
class wxExSTC;

/// Offers a wxExDebug that allows you to use an wxExSTC component as
/// graphical interface for a debug process (gdb).
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
  bool Execute(int item, wxExSTC* stc = nullptr);
  
  /// Sets the debug process, should be done before
  /// calling Execute.
  void SetDebugProcess(wxExProcess* process);
private:
  bool GetArgs(const std::string& command, std::string& args, wxExSTC* stc);
  std::vector < wxExMenuCommands <wxExMenuCommand > > m_Entries;
  
  /// Marker for a breakpoint.
  const wxExMarker m_Breakpoint = wxExMarker(2);
  
  wxExManagedFrame* m_Frame;
  wxExProcess* m_Debug = nullptr;
};
