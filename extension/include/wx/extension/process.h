////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <wx/extension/marker.h>

class wxTimer;
class wxExProcessImp;
class wxExShell;

/// Offers a process, capturing execution output depending
/// on sync or async call to Execute.
class WXDLLIMPEXP_BASE wxExProcess
{
public:
  /// Default constructor.
  wxExProcess();
  
  /// Destructor.
  virtual ~wxExProcess();

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
  ///   The STC component will be filled with output from the process.
  /// - In case synchronously (wxEXEC_SYNC) this call returns after execute ends, 
  ///   and the output is available using GetStdOut.
  /// Return value is false if process could not execute (and GetError is true), 
  /// or if config dialog was invoked and cancelled (and GetError is false).
  /// Normally the reason for not being able to run the process is logged
  /// by the wxWidgets library using a wxLogError. You can prevent that
  /// using a wxLogNull before calling Execute.
  bool Execute(
    /// command to be executed, if empty
    /// last given command is used
    const std::string& command = std::string(),
    /// flags for wxExecute
    int flags = wxEXEC_ASYNC,
    /// working dir, if empty last working dir is used
    const std::string& wd = std::string());
  
  /// Returns true if the command could not be executed.
  bool GetError() const {return m_Error;};

  /// Returns command executed.
  const auto & GetExecuteCommand() const {return m_Command;};

  /// Returns the shell component 
  /// (might be nullptr if PrepareOutput is not yet invoked).
  static auto* GetShell() {return m_Shell;};
  
  /// Returns the stderr.
  const auto & GetStdErr() const {return m_StdErr;};
  
  /// Returns the stdout.
  const auto & GetStdOut() const {return m_StdOut;};
  
  /// Returns true if this process is running.
  bool IsRunning() const;

  /// Kills the process.
  bool Kill();
  
  /// Construct the shell component.
  static void PrepareOutput(wxWindow* parent);

#if wxUSE_GUI
  /// Shows output from Execute (wxEXEC_SYNC) on the shell component.
  /// You can override this method to e.g. prepare a lexer on GetShell
  /// before calling this base method.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif

  /// Writes text to stdin of process.
  bool Write(const std::string& text);
private:
  void CheckInput();

  bool m_Error;

  wxCriticalSection m_Critical;
  std::string m_Command, m_StdErr, m_StdIn, m_StdOut;
  
  const wxExMarker m_MarkerSymbol = wxExMarker(2);

  static wxString m_WorkingDirKey;

#if wxUSE_GUI
  static wxExShell* m_Shell;
#endif  

  wxExProcessImp* m_Process;
  std::unique_ptr<wxTimer> m_Timer;
};
