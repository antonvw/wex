/******************************************************************************\
* File:          process.h
* Purpose:       Declaration of class 'wxExProcessWithListView'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_PROCESS_H
#define _EX_REPORT_PROCESS_H

#include <wx/process.h>

class wxExListViewFile;

/// Offers a wxProcess with output to a listview.
class wxExProcessWithListView : public wxProcess
{
public:
  /// Constructor.
  wxExProcessWithListView(
    wxExListViewFile* listview, 
    const wxString& command = wxEmptyString);

  /// Shows a config dialog, returns dialog return code.
  static int ConfigDialog();

  /// Executes the process asynchronously (this call immediately returns).
  /// The process output is added to the listview.
  /// The return value is the process id and zero value indicates 
  /// that the command could not be executed.
  long Execute();

  /// Is the process running.
  bool IsRunning() const;

  /// Returns whether a process has been selected.
  static bool IsSelected() {
    return !m_Command.empty();};

  /// Kills the process (sends specified signal if process still running).
  wxKillError Kill(wxSignal sig = wxSIGTERM);
protected:
  void OnTimer(wxTimerEvent& event);
private:
  // Checks whether input is available from process and updates the listview.
  // You should call it regularly.
  // This is done by the internal thread, so not for doxygen.
  void CheckInput();

  virtual void OnTerminate(int pid, int status); // overriden

  static wxString m_Command;
  wxExListViewFile* m_Owner;
  wxTimer m_Timer;

  DECLARE_EVENT_TABLE()
};
#endif
