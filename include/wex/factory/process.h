////////////////////////////////////////////////////////////////////////////////
// Name:      factory/process.h
// Purpose:   Declaration of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>

class wxEvtHandler;

namespace wex
{
namespace factory
{
class process_imp;

/// This class offers core process.
class process
{
  friend class process_imp;

public:
  /// Default constructor.
  process();

  /// Destructor, stops running process.
  virtual ~process();

  /// Virtual interface

  /// Starts the async process, collecting output
  /// into the out handler, and debug info in debug handler.
  /// You should have called set_handler_out or dbg before to set these.
  /// The output streams of the executing process are sent to these
  /// event handlers using wxPostEvent.
  /// Returns true if the async process is started.
  virtual bool async_system(
    const std::string& exe,
    const std::string& start_dir = std::string());

  // Writes data to the input of the async process.
  virtual bool write(const std::string& text);

  /// Other methods

  /// Returns last or current exe.
  const auto& get_exe() const { return m_exe; }

  /// Returns the stderr.
  const auto& get_stderr() const { return m_stderr; }

  /// Returns the stdout.
  const auto& get_stdout() const { return m_stdout; }

  /// Is this a debug process.
  bool is_debug() const;

  /// Is this process running.
  bool is_running() const;

  /// Sets debug event handler.
  void set_handler_dbg(wxEvtHandler* eh);

  /// Sets out event handler.
  void set_handler_out(wxEvtHandler* eh);

  // Stops the async process.
  bool stop();

  /// Runs the sync process, collecting output in stdout and stderr.
  /// It will execute the process and wait for it's exit,
  /// then return the exit_code.
  int system(
    const std::string& exe,
    const std::string& start_dir = std::string());

protected:
  std::string m_exe;

private:
  std::string m_stderr, m_stdout;

  wxEvtHandler* m_eh_debug{nullptr};
  wxEvtHandler* m_eh_out{nullptr};

  std::unique_ptr<process_imp> m_imp;
};
} // namespace factory
} // namespace wex
