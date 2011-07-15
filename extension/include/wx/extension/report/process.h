////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class 'wxExProcess'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_PROCESS_H
#define _EX_REPORT_PROCESS_H

#include <wx/process.h>

class wxExFrameWithHistory;
class wxExListView;

/// Offers a wxProcess with output to a listview.
class WXDLLIMPEXP_BASE wxExProcess : public wxProcess
{
public:
  /// Constructor.
  wxExProcess(
    wxExFrameWithHistory* frame, 
    const wxString& command = wxEmptyString);

  /// Shows a config dialog, sets the command 
  /// and returns dialog return code.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Select Process"));

  /// Executes the process asynchronously (this call immediately returns).
  /// The process output is added to the listview.
  /// The return value is the process id and zero value indicates 
  /// that the command could not be executed.
  /// When the process is finished, a ID_TERMINATED_PROCESS command event
  /// is sent to the frame as specified in the constructor.
  long Execute(const wxString& wd = wxEmptyString);
  
  /// Returns true if this process is running.
  bool IsRunning() const;

  /// Returns true if a process command is selected.
  static bool IsSelected() {
    return !m_Command.empty();};
    
  /// Kills the process (sends specified signal if process still running).
  wxKillError Kill(wxSignal sig = wxSIGKILL);
protected:
  virtual void OnTerminate(int pid, int status); // overriden
  void OnTimer(wxTimerEvent& event);
private:
  // Checks whether input is available from process and updates the listview.
  bool CheckInput();

  static wxString m_Command;
  static wxString m_WorkingDirKey;

  wxExFrameWithHistory* m_Frame;
  wxExListView* m_ListView;

  wxTimer m_Timer;

  DECLARE_EVENT_TABLE()
};
#endif
