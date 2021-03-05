////////////////////////////////////////////////////////////////////////////////
// Name:      process-core.h
// Purpose:   Declaration of class wex::core::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <atomic>
#include <boost/version.hpp>
#include <queue>
#if BOOST_VERSION / 100 % 1000 <= 65
#include <boost/asio.hpp>
#endif
#include <boost/process.hpp>

class wxEvtHandler;

namespace wex
{
  namespace core
  {
    /// This class offers core process.
    class process
    {
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

      // Stops the process.
      bool stop();

      /// Runs the sync process, collecting output in stdout and stderr.
      /// It will execute the process and wait for it's exit,
      /// then return the exit_code.
      int system(
        const std::string& exe,
        const std::string& start_dir = std::string());

      // Writes data to the input of the process.
      virtual bool write(const std::string& text);

    protected:
      std::string m_exe;

    private:
      bool        m_is_running{false};
      std::string m_stderr, m_stdout;

      std::atomic_bool m_debug{false};
#if BOOST_VERSION / 100 % 1000 <= 65
      std::shared_ptr<boost::asio::io_service> m_io;
#else
      std::shared_ptr<boost::asio::io_context> m_io;
#endif
      std::shared_ptr<std::queue<std::string>> m_queue;

      wxEvtHandler* m_eh_debug{nullptr};
      wxEvtHandler* m_eh_out{nullptr};

      boost::process::ipstream m_es, m_is;
      boost::process::opstream m_os;
      boost::process::group    m_group;
    };

  } // namespace core
} // namespace wex
