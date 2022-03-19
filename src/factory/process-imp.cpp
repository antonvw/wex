////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.cpp
// Purpose:   Implementation of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/core/log.h>
#include <wex/factory/defs.h>
#include <wex/factory/process.h>
#include <wx/event.h>

#include "process-imp.h"

#define WEX_POST(ID, TEXT, DEST)                         \
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID); \
  event.SetString(TEXT);                                 \
  wxPostEvent(DEST, event);

wex::factory::process_imp::process_imp()
  : m_io(std::make_shared<boost::asio::io_context>())
  , m_queue(std::make_shared<std::queue<std::string>>())
{
}

void wex::factory::process_imp::async_sleep_for(
  const std::chrono::milliseconds& ms)
{
  std::this_thread::sleep_for(ms);
}

void wex::factory::process_imp::async_system(
  process*            p,
  const process_data& data)
{
  m_debug.store(p->m_eh_debug != nullptr);
  m_data = data;

  boost_async_system(p, data);

  m_is_running.store(true);

  thread_input(p);
  thread_output(p);
  thread_error(p);
}

void wex::factory::process_imp::boost_async_system(
  process*            p,
  const process_data& data)
{
  bp::async_system(
    *m_io.get(),
    [this, p, data](boost::system::error_code error, int i)
    {
      m_is_running.store(false);

      log::debug("async_system") << "exit" << data.exe();

      if (m_debug.load())
      {
        WEX_POST(ID_DEBUG_EXIT, "", p->m_eh_debug)
      }
    },

    // clang-format off
    data.exe_path(),
    bp::args = data.args(),
    bp::start_dir = data.start_dir(),
    bp::std_err > m_es,
    bp::std_in < m_os, 
    bp::std_out > m_is,
    m_group);
  // clang-format on

  log::debug("async_system") << data.exe() << "debug:" << m_debug.load();
}

bool wex::factory::process_imp::stop()
{
  try
  {
    if (m_is_running.load())
    {
      if (m_group.valid())
      {
        m_group.terminate();
      }

      m_io->stop();
      m_is_running.store(false);
      return true;
    }
  }
  catch (std::exception& e)
  {
    log(e) << "stop" << m_data.exe();
  }

  return false;
}

void wex::factory::process_imp::thread_error(process* p)
{
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

void wex::factory::process_imp::thread_input(process* p)
{
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
}

void wex::factory::process_imp::thread_output(process* p)
{
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
}

bool wex::factory::process_imp::write(const std::string& text)
{
  if (text.empty() || !m_is_running.load())
  {
    return false;
  }

  m_queue->push(text);

  return true;
}
