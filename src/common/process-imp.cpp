////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "process-imp.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/log.h>
#include <wex/managed-frame.h>
#include <wex/process.h>
#include <wex/shell.h>

namespace bp = boost::process;

namespace wex
{
  int process_run_and_collect_output(
    const std::string& command,
    const std::string& cwd,
    std::string&       output,
    std::string&       error)
  {
    const std::string show_cwd(!cwd.empty() ? "cwd: " + cwd : std::string());

    try
    {
#if BOOST_VERSION / 100 % 1000 == 72
      const int ec = bp::system(bp::start_dir = cwd, command);
      error        = "boost version 1.72 error, please change version";
#else
      std::future<std::string> of, ef;

      const int ec = bp::system(
        bp::start_dir = cwd,
        command,
        bp::std_out > of,
        bp::std_err > ef);

      if (of.valid())
        output = of.get();
      if (ef.valid())
        error = ef.get();
#endif

      if (!ec)
      {
        log::debug("system") << command << show_cwd;
      }
      else
      {
        const std::string text(!error.empty() ? ":" + error : std::string());
        log("system") << command << show_cwd << "ec:" << ec << text;
      }

      return ec;
    }
    catch (std::exception& e)
    {
      log(e) << command << show_cwd;

      output.clear();
      error = e.what();
      return 1;
    }
    catch (...)
    {
      log("process_run_and_collect_output unknown exception") << command;
      return 1;
    }
  }
}; // namespace wex

#define WEX_POST(ID, TEXT, PROCESS)                        \
  if (PROCESS != nullptr)                                  \
  {                                                        \
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID); \
    event.SetString(TEXT);                                 \
    wxPostEvent(PROCESS, event);                           \
  }

wex::process_imp::process_imp(process* process)
  : m_process(process)
#if BOOST_VERSION / 100 % 1000 <= 65
  , m_io(std::make_shared<boost::asio::io_service>())
#else
  , m_io(std::make_shared<boost::asio::io_context>())
#endif
  , m_queue(std::make_shared<std::queue<std::string>>())
{
}

bool wex::process_imp::async(const std::string& path)
{
  try
  {
    bp::async_system(
      *m_io.get(),
      [&](boost::system::error_code error, int i) {
        log::debug("async") << "exit" << m_process->get_exec();
        if (m_debug.load())
        {
          WEX_POST(ID_DEBUG_EXIT, "", m_process->get_frame()->get_debug())
        }
      },
      bp::start_dir = path,
      m_process->get_exec(),
      bp::std_out > m_is,
      bp::std_in<m_os, bp::std_err> m_es,
      m_group);
  }
  catch (std::exception& e)
  {
    log(e) << m_process->get_exec() << "path:" << path;
    return false;
  }

  log::debug("async") << m_process->get_exec();

  m_debug.store(
    m_process->get_frame()->get_debug()->debug_entry().name() ==
    before(m_process->get_exec(), ' '));

  if (m_debug.load())
  {
    m_process->get_shell()->get_lexer().set(
      m_process->get_frame()->get_debug()->debug_entry().name());
  }
  else
  {
    m_process->get_shell()->SetFocus();
    m_process->get_frame()->show_process(true);
  }

  std::thread t([debug   = m_debug.load() && m_process->get_frame() != nullptr,
                 process = m_process,
                 &is     = m_is] {
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
        WEX_POST(
          ID_SHELL_APPEND,
          "\n*** LINE LIMIT ***\n",
          process->get_shell())
      }
      else if (isspace(text.back()) && !process->get_frame()->is_closing())
      {
        WEX_POST(ID_SHELL_APPEND, text, process->get_shell())

        if (text.back() == '\n')
        {
          linesize = 0;
        }

        if (debug)
        {
          line.append(text);

          if (line.back() == '\n')
          {
            WEX_POST(ID_DEBUG_STDOUT, line, process->get_frame()->get_debug())
            line.clear();
          }
        }

        text.clear();
      }
    }
  });
  t.detach();

  std::thread u([debug   = m_debug.load(),
                 io      = m_io,
                 &os     = m_os,
                 process = m_process,
                 queue   = m_queue] {
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

          if (debug && process->get_frame() != nullptr)
          {
            WEX_POST(ID_DEBUG_STDIN, text, process->get_frame()->get_debug())
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

  std::thread v([process = m_process, &es = m_es] {
    std::string text;

    while (es.good())
    {
      text.push_back(es.get());

      if (text.back() == '\n')
      {
        WEX_POST(ID_SHELL_APPEND_ERROR, text, process->get_shell())
        text.clear();
      }
    }
  });
  v.detach();

  return true;
}

bool wex::process_imp::stop()
{
  if (m_process == nullptr || m_io == nullptr || m_io->stopped())
  {
    return false;
  }

  log::trace("stop") << m_process->get_exec();

  try
  {
    if (m_group.valid())
    {
      m_group.terminate();
    }
    m_io->stop();
  }
  catch (std::exception& e)
  {
    log(e) << "stop" << m_process->get_exec();
  }

  return true;
}

bool wex::process_imp::write(const std::string& text)
{
  assert(!text.empty());

  if (m_process == nullptr || m_queue == nullptr || m_io->stopped())
  {
    return false;
  }

  if (!is_debug())
  {
    m_process->get_frame()->show_process(true);
  }

  m_queue->push(text);
  return true;
}
