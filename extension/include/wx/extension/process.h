////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <wx/process.h>
#include <wx/extension/marker.h>

class wxTimer;
class wxExShell;
class wxExSTC;

/// Offers a wxProcess, capturing execution output depending
/// on sync or async call to Execute.
class WXDLLIMPEXP_BASE wxExProcess : public wxProcess
{
public:
  /// Default constructor.
  wxExProcess();
  
  /// Destructor.
  virtual ~wxExProcess() {};

  /// Copy constructor.
  wxExProcess(const wxExProcess& process);

  /// Assignment operator.
  wxExProcess& operator=(const wxExProcess& p);

  /// Handles shell commands.
  bool Command(const wxString& command);
  
  /// Shows a config dialog, allowing you to set the command and folder.
  /// Returns dialog return code.
  static int ConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Select Process"),
    bool modal = true);
  
  /// Tries to execute debug action menu_id with data from stc.
  /// Returns false if e.g. debug process is not running.
  bool Debug(wxExSTC* stc, int menu_id);
  
  /// Executes the process.
  /// - In case asynchronously (wxEXEC_ASYNC) this call immediately returns.
  ///   The STC component will be filled with output from the process.
  /// - In case synchronously (wxEXEC_SYNC) this call returns after execute ends, 
  ///   and the output is available using GetOutput.
  /// Return value is false if process could not execute (and GetError is true), 
  /// or if config dialog was invoked and cancelled (and GetError is false).
  /// Normally the reason for not being able to run the process is logged
  /// by the wxWidgets library using a wxLogError. You can prevent that
  /// using a wxLogNull before calling Execute.
  bool Execute(
    /// command to be executed, if empty
    /// last given command is used
    const wxString& command = wxEmptyString,
    /// flags for wxExecute
    int flags = wxEXEC_ASYNC,
    /// working dir, if empty last working dir is used
    const wxString& wd = wxEmptyString);
  
  /// Returns command executed.
  const auto & GetExecuteCommand() const {return m_Command;};

  /// Returns true if the command could not be executed.
  bool GetError() const {return m_Error;};

  /// Returns the output from Execute (only filled for wxEXEC_SYNC).
  const auto & GetOutput() const {return m_Output;};
  
  /// Returns the shell component 
  /// (might be nullptr if PrepareOutput is not yet invoked).
  static auto* GetShell() {return m_Shell;};
  
  /// Returns true when the command executed resulted in stderr errors.
  bool HasStdError() const {return m_HasStdError;};
  
  /// Returns true if this process is running.
  bool IsRunning() const;

  /// Kills the process (sends specified signal if process still running).
  wxKillError Kill(wxSignal sig = wxSIGKILL);
  
  /// Construct the shell component.
  static void PrepareOutput(wxWindow* parent);

#if wxUSE_GUI
  /// Shows output from Execute (wxEXEC_SYNC) on the shell component.
  /// You can override this method to e.g. prepare a lexer on GetShell
  /// before calling this base method.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif
protected:
  virtual void OnTerminate(int pid, int status) override;
private:
  void CheckInput();

  bool m_Error, m_HasStdError;

  wxCriticalSection m_Critical;
  wxString m_Command, m_Input, m_Output;
  
  const wxExMarker m_MarkerSymbol = wxExMarker(2);

  static wxString m_WorkingDirKey;

#if wxUSE_GUI
  static wxExShell* m_Shell;
#endif  

  std::unique_ptr<wxTimer> m_Timer;
};
