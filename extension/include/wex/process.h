////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <signal.h>  
#include <wex/window-data.h>

namespace wex
{
  class managed_frame;
  class process_imp;
  class shell;

  /// Offers a process, capturing execution output.
  class process
  {
  public:
    /// process execute type
    /// - default this call immediately returns.
    ///   The stc component will be filled with output from the process.
    /// - if EXEC_WAIT this call returns after execute ends, 
    ///   and the output is available using get_stdout.
    enum 
    {
      EXEC_DEFAULT = 0, ///< default
      EXEC_WAIT    = 1, ///< wait for process finish
    };

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
    static int config_dialog(const window_data& data = window_data());
    
    /// Returns true if the command could not be executed.
    bool error() const {return m_Error;};

    /// Executes the process.
    /// Return value is false if process could not execute (and error is true), 
    /// or if config dialog was invoked and cancelled (and error is false).
    bool execute(
      /// command to be executed, if empty
      /// last given command is used
      const std::string& command = std::string(),
      /// process excute flags
      int type = EXEC_DEFAULT,
      /// working dir, if empty last working dir is used
      const std::string& wd = std::string());
    
    /// Returns command executed.
    const auto & get_command_executed() const {return m_Command;};

    /// Returns the shell component 
    /// (might be nullptr if prepare_output is not yet invoked).
    static auto* get_shell() {return m_Shell;};
    
    /// Returns the stderr.
    const auto & get_stderr() const {return m_StdErr;};
    
    /// Returns the stdout.
    const auto & get_stdout() const {return m_StdOut;};
    
    /// Returns true if this process is running.
    bool is_running() const;

    /// Sends specified signal to this process.
    /// Returns true if signalling is ok.
    /// (SIGKILL is not ISO C99 and not known by windows).
    bool kill(
  #ifdef __UNIX__
      int sig = SIGKILL);
  #else
      int sig = SIGTERM);
  #endif
    
    /// Sends specified signal to all processes that are still running.
    /// Returns the number of processes signalled.
    /// (SIGKILL is not ISO C99 and not known by windows).
    static int kill_all(
  #ifdef __UNIX__
      int sig = SIGKILL);
  #else
      int sig = SIGTERM);
  #endif
    
    /// Construct the shell component.
    static void prepare_output(wxWindow* parent);

    /// Shows std output from Execute on the shell component.
    /// You can override this method to e.g. prepare a lexer on get_shell
    /// before calling this base method.
    virtual void show_output(const std::string& caption = std::string()) const;

    /// Writes text to stdin of process.
    bool write(const std::string& text);
  private:
    bool m_Error {false};

    std::string m_Command, m_StdErr, m_StdOut;
    
    static std::string m_WorkingDirKey;

    static inline shell* m_Shell = nullptr;

    std::unique_ptr<process_imp> m_Process;
    
    managed_frame* m_Frame;
  };
};
