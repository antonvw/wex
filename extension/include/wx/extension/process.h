////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <signal.h>  
#include <wx/extension/window-data.h>

namespace wex
{
  class managed_frame;
  class process_imp;
  class shell;

  /// process excute flags
  /// - default this call immediately returns.
  ///   The STC component will be filled with output from the process.
  /// - if PROCESS_EXEC_WAIT this call returns after execute ends, 
  ///   and the output is available using GetStdOut.
  enum 
  {
    PROCESS_EXEC_DEFAULT = 0x0000, ///< default flags
    PROCESS_EXEC_WAIT    = 0x0001, ///< wait for process finish
  };

  /// Offers a process, capturing execution output.
  class process
  {
  public:
    /// Default constructor.
    process();
    
    /// Destructor.
    virtual ~process();

    /// Copy constructor.
    process(const process& process);

    /// Assignment operator.
    process& operator=(const process& p);

    /// Shows a config dialog, allowing you to set the command and folder.
    /// Returns dialog return code.
    static int ConfigDialog(const window_data& data = window_data());
    
    /// Executes the process.
    /// Return value is false if process could not execute (and GetError is true), 
    /// or if config dialog was invoked and cancelled (and GetError is false).
    bool Execute(
      /// command to be executed, if empty
      /// last given command is used
      const std::string& command = std::string(),
      /// process excute flags
      long flags = PROCESS_EXEC_DEFAULT,
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

    /// Sends specified signal to this process.
    /// Returns true if signalling is ok.
    /// (SIGKILL is not ISO C99 and not known by windows).
    bool Kill(
  #ifdef __UNIX__
      int sig = SIGKILL);
  #else
      int sig = SIGTERM);
  #endif
    
    /// Sends specified signal to all processes that are still running.
    /// Returns the number of processes signalled.
    /// (SIGKILL is not ISO C99 and not known by windows).
    static int KillAll(
  #ifdef __UNIX__
      int sig = SIGKILL);
  #else
      int sig = SIGTERM);
  #endif
    
    /// Construct the shell component.
    static void PrepareOutput(wxWindow* parent);

    /// Shows std output from Execute on the shell component.
    /// You can override this method to e.g. prepare a lexer on GetShell
    /// before calling this base method.
    virtual void ShowOutput(const std::string& caption = std::string()) const;

    /// Writes text to stdin of process.
    bool Write(const std::string& text);
  private:
    bool m_Error {false};

    std::string m_Command, m_StdErr, m_StdOut;
    
    static std::string m_WorkingDirKey;

    static inline shell* m_Shell = nullptr;

    std::unique_ptr<process_imp> m_Process;
    
    managed_frame* m_Frame;
  };
};
