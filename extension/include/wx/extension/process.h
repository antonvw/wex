////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_PROCESS_H
#define _EX_PROCESS_H

#include <wx/process.h>
#include <wx/timer.h>
#include <wx/extension/stcdlg.h>

/// Offers a wxProcess, capturing execution output depending
/// on sync ar async call to Execute.
class WXDLLIMPEXP_BASE wxExProcess : public wxProcess
{
public:
  /// Default constructor.
  wxExProcess();
  
  /// Destructor.
 ~wxExProcess();

  /// Copy constructor.
  wxExProcess(const wxExProcess& process);

  /// Assignment operator.
  wxExProcess& operator=(const wxExProcess& p);

  /// Shows a config dialog, allowing you to set the command and folder.
  /// Returns dialog return code.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Select Process"),
    bool modal = true);

  /// Executes the process.
  /// - In case asynchronously (wxEXEC_ASYNC) this call immediately returns.
  ///   The dialog will be shown, with STC component shell enabled,
  ///   and will be filled with output from the process.
  ///   The return value is the process id and zero value indicates 
  ///   that the command could not be executed.
  ///   When the process is finished, a ID_TERMINATED_PROCESS command event
  ///   is sent to the application top window.
  /// - In case synchronously (wxEXEC_SYNC) this call returns after execute ends, 
  ///   and the output is collected in the output member.
  ///   The dialog is not shown (call ShowOutput). 
  ///   The STC component in the dialog will be shell disabled.
  ///   Return value is return value from wxExecute, -1 execute fails.
  /// The return value can also be -2, if config dialog was invoked and
  /// cancelled.
  long Execute(
    /// command to be executed, if empty
    /// last given command is used
    const wxString& command = wxEmptyString,
    /// flags for wxExecute
    int flags = wxEXEC_ASYNC,
    /// working dir, if empty last working dir is used
    const wxString& wd = wxEmptyString);
  
  /// Returns true if the command could not be executed.
  bool GetError() const {return m_Error;};

  /// Gets the output from Execute (only filled for wxEXEC_SYNC).
  const wxString& GetOutput() const {return m_Output;};
  
  /// Returns the STC component from the dialog
  /// (might be NULL if dialog is NULL).
  static wxExSTC* GetSTC() {
    return m_Dialog != NULL ? m_Dialog->GetSTC(): NULL;};
    
  /// Returns true if this process is running.
  bool IsRunning() const;

  /// Returns true if a process command is selected.
  bool IsSelected() const {return !m_Command.empty();};
    
  /// Kills the process (sends specified signal if process still running).
  wxKillError Kill(wxSignal sig = wxSIGKILL);
  
#if wxUSE_GUI
  /// Shows output from Execute (wxEXEC_SYNC) on the STC component 
  /// (if no error code returned from Execute).
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif
protected:
  /// Overriden from wxProcess.
  virtual void OnTerminate(int pid, int status);
  
  /// Handles shell events.
  void OnCommand(wxCommandEvent& event);
  
  /// Handles timer events.
  void OnTimer(wxTimerEvent& event);
private:
  void CheckInput();

  bool m_Busy;
  bool m_Error;
  bool m_Sync;

  wxString m_Command;  
  wxString m_Output;
  
  static wxString m_WorkingDirKey;

#if wxUSE_GUI
  static wxExSTCEntryDialog* m_Dialog;
#endif  

  wxTimer* m_Timer;

  DECLARE_EVENT_TABLE()
};

#endif
