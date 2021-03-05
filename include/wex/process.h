////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <wex/process-core.h>
#include <wex/window-data.h>

namespace wex
{
  class managed_frame;
  class shell;

  /// Offers a process, capturing execution output.
  class process : public core::process
  {
  public:
    /// process execute type
    /// - EXEC_NO_WAIT this call immediately returns.
    ///   The stc component will be filled with output from the process.
    /// - EXEC_WAIT this call returns after execute ends,
    ///   and the output is available using get_stdout.
    enum exec_t
    {
      EXEC_NO_WAIT, ///< do not wait
      EXEC_WAIT,    ///< wait for process finish
    };

    /// Static interface.

    /// Shows a config dialog, allowing you to set the command and folder.
    /// Returns dialog return code.
    static int config_dialog(const data::window& data = data::window());

    /// Returns the shell component
    /// (might be nullptr if prepare_output is not yet invoked).
    static auto* get_shell() { return m_shell; };

    /// Construct the shell component, and returns it.
    static shell* prepare_output(wxWindow* parent);

    /// Other methods.

    /// Default constructor.
    process();

    /// Destructor.
    ~process() override;

    /// Copy constructor.
    process(const process& process);

    /// Assignment operator.
    process& operator=(const process& p);

    /// Executes the process.
    /// Return value is false if process could not execute,
    /// or if config dialog was invoked and cancelled.
    bool execute(
      /// command to be executed, if empty
      /// last given command is used
      const std::string& exe = std::string(),
      /// process execute type
      exec_t type = EXEC_NO_WAIT,
      /// start dir, if empty last start dir is used
      const std::string& start_dir = std::string());

    /// Returns the frame.
    auto* get_frame() { return m_frame; };

    /// Shows stdout or stderr from execute on the shell component.
    /// You can override this method to e.g. prepare a lexer on get_shell
    /// before calling this base method.
    virtual void show_output(const std::string& caption = std::string()) const;

    /// Writes text to stdin of process.
    /// Default the response stdout is collected in the shell,
    bool write(const std::string& text) override;

  private:
    static std::string   m_working_dir_key;
    static inline shell* m_shell = nullptr;

    managed_frame* m_frame;
  };
}; // namespace wex
