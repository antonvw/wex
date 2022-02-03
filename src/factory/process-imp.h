////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.h
// Purpose:   Declaration of class wex::factory::process_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <atomic>
#include <queue>

#include <boost/process.hpp>

namespace bp = boost::process;

namespace wex::factory
{
class process;

/// This class offers methods to support processing. It uses
/// the boost::process classes.
class process_imp
{
public:
  /// Default constructor.
  process_imp();

  /// Sleeps for some milliseconds time.
  void async_sleep_for(const std::chrono::milliseconds& ms);

  /// Runs the exe as a async process.
  void async_system(process* p, const process_data& data);

  /// Returns the exe.
  const auto& exe() const { return m_exe; }

  /// Returns true if this is a debug process.
  bool is_debug() const { return m_debug; }

  /// Returns true if process is running.
  bool is_running() const { return m_is_running; }

  /// Stops the async process.
  bool stop();

  /// Writes text to the proess.
  bool write(const std::string& text);

private:
  std::shared_ptr<boost::asio::io_context> m_io;
  std::shared_ptr<std::queue<std::string>> m_queue;

  std::atomic<bool> m_debug{false};
  std::atomic<bool> m_is_running{false};

  std::string m_exe;

  bp::ipstream m_es, m_is;
  bp::opstream m_os;
  bp::group    m_group;
};
}; // namespace wex::factory
