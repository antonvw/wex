////////////////////////////////////////////////////////////////////////////////
// Name:      process-core.h
// Purpose:   Declaration of class wex::core::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>
#include <memory>
#include <queue>

class wxEvtHandler;

namespace wex
{
  namespace core
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

      /// Starts the async process, collecting output
      /// into the out handler, and debug info in debug handler.
      /// You should have called set_handler_out or dbg before to set these.
      /// The output streams of the executing process are sent to these
      /// event handlers using wxPostEvent.
      /// Returns true if the async process is started.
      bool async(
        const std::string& exe,
        const std::string& start_dir = std::string());

      /// Returns last or current exe.
      const auto& get_exe() const { return m_exe; };

      /// Returns the stderr.
      const auto& get_stderr() const { return m_stderr; };

      /// Returns the stdout.
      const auto& get_stdout() const { return m_stdout; };

      /// Is this a debug process.
      bool is_debug() const { return m_debug.load(); };

      /// Is this process running.
      bool is_running() const { return m_is_running; };

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

      // Writes data to the input of the async process.
      virtual bool write(const std::string& text);

    protected:
      std::string m_exe;

    private:
      bool        m_is_running{false};
      std::string m_stderr, m_stdout;

      wxEvtHandler* m_eh_debug{nullptr};
      wxEvtHandler* m_eh_out{nullptr};

      std::atomic_bool             m_debug{false};
      std::unique_ptr<process_imp> m_imp;
    };
  } // namespace core
} // namespace wex
