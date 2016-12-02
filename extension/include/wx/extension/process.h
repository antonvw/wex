////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcess
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

class wxExManagedFrame;
class wxExProcessImp;
class wxExShell;

/// Offers a process, capturing execution output.
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
  /// Return value is false if process could not execute (and GetError is true), 
  /// or if config dialog was invoked and cancelled (and GetError is false).
  bool Execute(
    /// command to be executed, if empty
    /// last given command is used
    const std::string& command = std::string(),
    /// wait for process finished
    /// - if false this call immediately returns.
    ///   The STC component will be filled with output from the process.
    /// - if true this call returns after execute ends, 
    ///   and the output is available using GetStdOut.
    bool wait = false,
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

  /// Kills this process.
  /// Returns true if process is killed.
  bool Kill();
  
  /// Kills all processes that are still running.
  /// Returns the number of processes killed.
  static int KillAll();
  
  /// Construct the shell component.
  static void PrepareOutput(wxWindow* parent);

#if wxUSE_GUI
  /// Shows std output from Execute on the shell component.
  /// You can override this method to e.g. prepare a lexer on GetShell
  /// before calling this base method.
  virtual void ShowOutput(const wxString& caption = wxEmptyString) const;
#endif

  /// Writes text to stdin of process.
  bool Write(const std::string& text);
private:
  bool m_Error = false;

  std::string m_Command, m_StdErr, m_StdOut;
  
  static wxString m_WorkingDirKey;

#if wxUSE_GUI
  static wxExShell* m_Shell;
#endif  

  std::unique_ptr<wxExProcessImp> m_Process;
  
  wxExManagedFrame* m_Frame;
};
