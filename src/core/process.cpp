////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::core::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>
#if BOOST_VERSION / 100 % 1000 <= 65
#include <boost/asio.hpp>
#endif
#include <boost/process.hpp>

#include <algorithm>
#include <atomic>
#include <thread>
#include <wex/defs.h>
#include <wex/log.h>
#include <wex/process-core.h>
#include <wx/event.h>

namespace bp = boost::process;

namespace wex::core
{
  class process_imp
  {
  public:
    process_imp()
#if BOOST_VERSION / 100 % 1000 <= 65
      : m_io(std::make_shared<boost::asio::io_service>())
#else
      : m_io(std::make_shared<boost::asio::io_context>())
#endif
      , m_queue(std::make_shared<std::queue<std::string>>())
    {
      ;
    }

    void
    async(const std::string& exe, const std::string& start_dir, process* p);

    void stop()
    {
      if (m_group.valid())
      {
        m_group.terminate();
      }

      m_io->stop();
    }

    void write(const std::string& text) { m_queue->push(text); }

  private:
#if BOOST_VERSION / 100 % 1000 <= 65
    std::shared_ptr<boost::asio::io_service> m_io;
#else
    std::shared_ptr<boost::asio::io_context> m_io;
#endif
    std::shared_ptr<std::queue<std::string>> m_queue;

    bp::ipstream m_es, m_is;
    bp::opstream m_os;
    bp::group    m_group;
  };
}; // namespace wex::core

wex::core::process::process()
  : m_imp(std::make_unique<process_imp>())
{
}

wex::core::process::~process()
{
  stop();
}

bool wex::core::process::async(
  const std::string& exe,
  const std::string& start_dir)
{
  if (m_eh_out == nullptr)
  {
    return false;
  }

  try
  {
    m_imp->async(exe, start_dir, this);
    return true;
  }
  catch (std::exception& e)
  {
    log(e) << exe << "start_dir:" << start_dir;
    return false;
  }
}

void wex::core::process::set_handler_dbg(wxEvtHandler* eh)
{
  m_eh_debug = eh;
  m_debug.store(m_eh_debug != nullptr);
}

void wex::core::process::set_handler_out(wxEvtHandler* eh)
{
  m_eh_out = eh;
}

bool wex::core::process::stop()
{
  if (!m_is_running)
  {
    return false;
  }

  try
  {
    m_imp->stop();
  }
  catch (std::exception& e)
  {
    log(e) << "stop" << m_exe;
  }

  m_is_running = false;

  return true;
}

int wex::core::process::system(
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

bool wex::core::process::write(const std::string& text)
{
  if (text.empty() || !m_is_running)
  {
    return false;
  }

  m_imp->write(text);

  return true;
}

#define WEX_POST(ID, TEXT, DEST)                         \
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID); \
  event.SetString(TEXT);                                 \
  wxPostEvent(DEST, event);

void wex::core::process_imp::async(
  const std::string& exe,
  const std::string& start_dir,
  process*           p)
{
  bp::async_system(
    *m_io.get(),
    [&](boost::system::error_code error, int i) {
      p->m_is_running = false;
      log::debug("async") << "exit" << exe;
      if (p->m_debug.load())
      {
        WEX_POST(ID_DEBUG_EXIT, "", p->m_eh_debug)
      }
    },
    bp::start_dir = start_dir,
    exe,
    bp::std_out > m_is,
    bp::std_in<m_os, bp::std_err> m_es,
    m_group);

  log::debug("async") << exe;

  p->m_exe        = exe;
  p->m_is_running = true;

  std::thread t([debug = p->m_debug.load(),
                 &dbg  = p->m_eh_debug,
                 &out  = p->m_eh_out,
                 &is   = m_is] {
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

  std::thread u([debug = p->m_debug.load(),
                 io    = m_io,
                 &os   = m_os,
                 dbg   = p->m_eh_debug,
                 queue = m_queue] {
    while (os.good() && !io->stopped())
    {
#if BOOST_VERSION / 100 % 1000 > 65
      io->run_one_for(std::chrono::milliseconds(10));
#else
      io->run_one();
#endif

      if (!queue->empty())
      {
        const auto& text(queue->front());

        if (os.good() && !io->stopped())
        {
          log::debug("async") << "write:" << text;

          os << text << std::endl;

          if (debug)
          {
            WEX_POST(ID_DEBUG_STDIN, text, dbg)
          }
        }
        else
        {
          log::debug("async") << "skip:" << text;
        }

        queue->pop();
      }
    }
  });
  u.detach();

  std::thread v([out = p->m_eh_out, &es = m_es] {
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
