////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.h
// Purpose:   Declaration of class wex::factory::process_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <queue>

#include <boost/process/v1/args.hpp>
#include <boost/process/v1/async_system.hpp>
#include <boost/process/v1/group.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#include <boost/process/v1/start_dir.hpp>
#include <boost/process/v1/system.hpp>

namespace bp = boost::process::v1;

class wxEvtHandler;

namespace wex::factory
{
class process;

/// This class offers methods to support processing. It uses
/// the boost::process::v1 classes.
class process_imp
{
public:
  /// Default constructor.
  process_imp();

  /// Sleeps for some milliseconds time.
  void async_sleep_for(const std::chrono::milliseconds& ms);

  /// Runs the exe as a async process.
  void async_system(process* p);

  /// Returns true if this is a debug process.
  bool is_debug() const { return m_debug; }

  /// Returns true if process is running.
  bool is_running() const { return m_is_running; }

  /// Stops the async process.
  bool stop(wxEvtHandler* e);

  /// Writes text to the proess.
  bool write(const std::string& text);

private:
  void boost_async_system(process* p);
  void thread_error(const process* p);
  void thread_input(const process* p);
  void thread_output(const process* p);

  std::shared_ptr<boost::asio::io_context> m_io;
  std::shared_ptr<std::queue<std::string>> m_queue;

  std::atomic<bool> m_debug{false};
  std::atomic<bool> m_is_running{false};

  bp::ipstream m_es, m_is;
  bp::opstream m_os;
  bp::group    m_group;
};
}; // namespace wex::factory
