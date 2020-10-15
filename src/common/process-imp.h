////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.h
// Purpose:   Declaration of class wex::process_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
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

namespace wex
{
  class process;

  /// This class offers our process implementation.
  class process_imp
  {
  public:
    /// Constructor.
    process_imp(process* process);

    // Starts the async process, collecting output
    // into the stc shell of the parent process.
    bool async(const std::string& path);

    /// Is this a debug process.
    bool is_debug() const { return m_debug.load(); };

    /// Is this process running.
    bool is_running() const { return !m_io->stopped(); };

    // Stops the process.
    bool stop();

    // Writes data to the input of the process.
    bool write(const std::string& text);

  private:
    std::atomic_bool m_debug{false};
#if BOOST_VERSION / 100 % 1000 <= 65
    std::shared_ptr<boost::asio::io_service> m_io;
#else
    std::shared_ptr<boost::asio::io_context> m_io;
#endif
    std::shared_ptr<std::queue<std::string>> m_queue;

    process*                 m_process;
    boost::process::ipstream m_es, m_is;
    boost::process::opstream m_os;
    boost::process::group    m_group;
  };

  int process_run_and_collect_output(
    const std::string& command,
    const std::string& cwd,
    std::string&       output,
    std::string&       error);
} // namespace wex
