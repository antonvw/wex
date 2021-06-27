////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/process.hpp>
#include <boost/version.hpp>

#include <algorithm>
#include <atomic>
#include <queue>
#include <thread>
#include <wex/defs.h>
#include <wex/factory/process.h>
#include <wex/log.h>
#include <wx/event.h>

namespace bp = boost::process;

namespace wex::factory
{
class process_imp
{
public:
  process_imp()
    : m_io(std::make_shared<boost::asio::io_context>())
    , m_queue(std::make_shared<std::queue<std::string>>())
  {
    ;
  }

  void async_system(
    const std::string& exe,
    const std::string& start_dir,
    process*           p);

  bool is_debug() const { return m_debug; }

  bool is_running() const { return m_is_running; }

  bool stop()
  {
    if (!m_is_running.load())
    {
      return false;
    }

    if (m_group.valid())
    {
      m_group.terminate();
    }

    m_io->stop();
    m_is_running.store(false);
    return true;
  }

  bool write(const std::string& text)
  {
    if (text.empty() || !m_is_running.load())
    {
      return false;
    }
    m_queue->push(text);
    return true;
  }

private:
  std::shared_ptr<boost::asio::io_context> m_io;
  std::shared_ptr<std::queue<std::string>> m_queue;

  std::atomic_bool m_debug{false};
  std::atomic_bool m_is_running{false};

  bp::ipstream m_es, m_is;
  bp::opstream m_os;
  bp::group    m_group;
};
}; // namespace wex::factory

wex::factory::process::process()
  : m_imp(std::make_unique<process_imp>())
{
}

wex::factory::process::~process()
{
  stop();
}

bool wex::factory::process::async_system(
  const std::string& exe,
  const std::string& start_dir)
{
  if (m_eh_out == nullptr)
  {
    return false;
  }

  try
  {
    m_imp->async_system(exe, start_dir, this);
    return true;
  }
  catch (std::exception& e)
  {
    log(e) << exe << "start_dir:" << start_dir;
    return false;
  }
}

bool wex::factory::process::is_debug() const
{
  return m_imp->is_debug();
}

bool wex::factory::process::is_running() const
{
  return m_imp->is_running();
}

void wex::factory::process::set_handler_dbg(wxEvtHandler* eh)
{
  m_eh_debug = eh;
}

void wex::factory::process::set_handler_out(wxEvtHandler* eh)
{
  m_eh_out = eh;
}

bool wex::factory::process::stop()
{
  try
  {
    return m_imp->stop();
  }
  catch (std::exception& e)
  {
    log(e) << "stop" << m_exe;
  }

  return true;
}

int wex::factory::process::system(
  const std::string& exe,
  const std::string& start_dir)
{
  m_exe = exe;

  const std::string log_dir(
    !start_dir.empty() ? "start_dir: " + start_dir : std::string());

  try
  {
#if BOOST_VERSION / 100 % 1000 == 72
    const int ec = bp::system(bp::start_dir = start_dir, exe);
    error        = "boost version 1.72 error, please change version";
#else
    std::future<std::string> of, ef;

    const int ec = bp::system(
      bp::start_dir = start_dir,
      exe,
      bp::std_out > of,
      bp::std_err > ef);

    if (of.valid())
      m_stdout = of.get();
    if (ef.valid())
      m_stderr = ef.get();
#endif

    if (!ec)
    {
      log::debug("system") << exe << log_dir;
    }
    else
    {
      const std::string text(
        !m_stderr.empty() ? ":" + m_stderr : std::string());
      log("system") << exe << log_dir << "ec:" << ec << text;
    }

    return ec;
  }
  catch (std::exception& e)
  {
    log(e) << exe << log_dir;

    m_stdout.clear();
    m_stderr = e.what();
    return 1;
  }
  catch (...)
  {
    log("system unknown exception") << exe;
    return 1;
  }
}

bool wex::factory::process::write(const std::string& text)
{
  return m_imp->write(text);
}

#define WEX_POST(ID, TEXT, DEST)                         \
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID); \
  event.SetString(TEXT);                                 \
  wxPostEvent(DEST, event);

void wex::factory::process_imp::async_system(
  const std::string& exe,
  const std::string& start_dir,
  process*           p)
{
  m_debug.store(p->m_eh_debug != nullptr);

  bp::async_system(
    *m_io.get(),
    [&](boost::system::error_code error, int i)
    {
      m_is_running.store(false);

      log::debug("async_system") << "exit" << exe;

      if (m_debug.load())
      {
        WEX_POST(ID_DEBUG_EXIT, "", p->m_eh_debug)
      }
    },

    bp::start_dir = start_dir,
    exe,
    bp::std_out > m_is,
    bp::std_in<m_os, bp::std_err> m_es,
    m_group);

  log::debug("async_system") << exe << m_debug.load();

  p->m_exe = exe;
  m_is_running.store(true);

  std::thread t(
    [debug = m_debug.load(),
     &dbg  = p->m_eh_debug,
     &out  = p->m_eh_out,
     &is   = m_is]
    {
      std::string text, line;
      line.reserve(1000000);
      text.reserve(1000000);
      int  linesize = 0;
      bool error    = false;

      while (is.good() && !error)
      {
        text.push_back(is.get());
        linesize++;

        if (linesize > 10000)
        {
          error = true;
          WEX_POST(ID_SHELL_APPEND, "\n*** LINE LIMIT ***\n", out)
        }
        else if (isspace(text.back()))
        {
          WEX_POST(ID_SHELL_APPEND, text, out)

          if (text.back() == '\n')
          {
            linesize = 0;
          }

          if (debug)
          {
            line.append(text);

            if (line.back() == '\n')
            {
              WEX_POST(ID_DEBUG_STDOUT, line, dbg)
              line.clear();
            }
          }

          text.clear();
        }
      }
    });
  t.detach();

  std::thread u(
    [debug = m_debug.load(),
     io    = m_io,
     &os   = m_os,
     dbg   = p->m_eh_debug,
     queue = m_queue]
    {
      while (os.good() && !io->stopped())
      {
        io->run_one_for(std::chrono::milliseconds(10));

        if (!queue->empty())
        {
          const auto& text(queue->front());

          if (os.good() && !io->stopped())
          {
            log::debug("async_system") << "write:" << text;

            os << text << std::endl;

            if (debug)
            {
              WEX_POST(ID_DEBUG_STDIN, text, dbg)
            }
          }
          else
          {
            log::debug("async_system") << "skip:" << text;
          }

          queue->pop();
        }
      }
    });
  u.detach();

  std::thread v(
    [out = p->m_eh_out, &es = m_es]
    {
      std::string text;

      while (es.good())
      {
        text.push_back(es.get());

        if (text.back() == '\n')
        {
          WEX_POST(ID_SHELL_APPEND_ERROR, text, out)
          text.clear();
        }
      }
    });
  v.detach();
}
