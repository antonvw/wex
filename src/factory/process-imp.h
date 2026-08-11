////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.h
// Purpose:   Declaration of class wex::factory::process_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <atomic>
#include <queue>

#include <boost/process/process.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/readable_pipe.hpp>

namespace ba = boost::asio;
namespace bp = boost::process;

class wxEvtHandler;

namespace wex::factory
{
class process;

/// This class offers methods to support processing. It uses
/// the boost::asio classes.
class process_imp
{
public:
  /// Default constructor.
  process_imp();

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

  ba::readable_pipe m_ep, m_ip;
  ba::writable_pipe m_op;
};
}; // namespace wex::factory
