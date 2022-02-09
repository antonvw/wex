////////////////////////////////////////////////////////////////////////////////
// Name:      factory/process.h
// Purpose:   Declaration of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <wex/factory/process-data.h>

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
  virtual bool async_system(const process_data& data);

  // Writes data to the input of the async process.
  virtual bool write(const std::string& text);

  /// Other methods

  /// Sleeps for some milliseconds time.
  void async_sleep_for(const std::chrono::milliseconds& ms);

  /// Returns last or current async exe.
  const std::string exe() const;

  /// Is this a debug process.
  bool is_debug() const;

  /// Is this process running.
  bool is_running() const;

  /// Sets debug event handler.
  void set_handler_dbg(wxEvtHandler* eh);

  /// Sets out event handler.
  void set_handler_out(wxEvtHandler* eh);

  /// Returns the stderr.
  const auto& std_err() const { return m_stderr; }

  /// Returns the stdout.
  const auto& std_out() const { return m_stdout; }

  // Stops the async process.
  bool stop();

  /// Runs the sync process, collecting output in stdout and stderr.
  /// It will execute the process and wait for it's exit,
  /// then return the exit_code.
  int system(const process_data& data);

private:
  std::string m_stderr, m_stdout;

  wxEvtHandler *m_eh_debug{nullptr}, *m_eh_out{nullptr};

  std::unique_ptr<process_imp> m_imp;
};
} // namespace factory
} // namespace wex
