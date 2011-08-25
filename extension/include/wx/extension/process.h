////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_PROCESS_H
#define _EX_PROCESS_H

#include <wx/process.h>

class wxExSTCShell;

/// Offers a wxProcess with default output to a wxExSTCShell.
/// You can change that by inheriting this class and provide
/// your own report generators.
class WXDLLIMPEXP_BASE wxExProcess : public wxProcess
{
public:
  /// Default constructor.
  wxExProcess(const wxString& command = wxEmptyString);

  /// Shows a config dialog, sets the command 
  /// and returns dialog return code.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Select Process"));

  /// Executes the process asynchronously (this call immediately returns).
  /// For each output line ReportAdd is invoked.
  /// The return value is the process id and zero value indicates 
  /// that the command could not be executed.
  /// When the process is finished, a ID_TERMINATED_PROCESS command event
  /// is sent to the application top window.
  long Execute(const wxString& wd = wxEmptyString);
  
  /// Returns true if this process is running.
  bool IsRunning() const;

  /// Returns true if a process command is selected.
  static bool IsSelected() {
    return !m_Command.empty();};
    
  /// Kills the process (sends specified signal if process still running).
  wxKillError Kill(wxSignal sig = wxSIGKILL);
protected:
  /// Overriden from wxProcess.
  virtual void OnTerminate(int pid, int status);
  
  /// Override to add a process output line to report.
  /// Default puts the message on the STC shell.
  virtual void ReportAdd(
    /// complete line (not empty)
    const wxString& line, 
    /// path from line (if available)
    const wxString& path,
    /// lineno from line (if available)
    const wxString& lineno);
    
  /// Override to create a report.
  /// Called for each invocation of Execute.
  /// Default report creator uses a wxExSTShell.
  virtual void ReportCreate();
    
  void OnTimer(wxTimerEvent& event);
private:
  bool CheckInput();

  static wxString m_Command;
  static wxString m_WorkingDirKey;

#if wxUSE_GUI
  wxExSTCShell* m_Shell;
#endif  

  wxTimer m_Timer;

  DECLARE_EVENT_TABLE()
};

#endif
