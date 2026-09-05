////////////////////////////////////////////////////////////////////////////////
// Name:      process-imp.cpp
// Purpose:   Implementation of class wex::factory::process_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/asio.hpp>
#include <boost/process/start_dir.hpp>
#include <boost/process/stdio.hpp>

#include <thread>
#include <wex/core/log.h>
#include <wex/factory/defs.h>
#include <wex/factory/process.h>
#include <wx/event.h>

#include "process-imp.h"

#define WEX_POST(ID, TEXT, DEST)                                               \
  if (DEST != nullptr)                                                         \
  {                                                                            \
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID);                     \
    event.SetString(TEXT);                                                     \
    wxPostEvent(DEST, event);                                                  \
  }

namespace wex::factory
{
bool read_from_pipe(ba::readable_pipe& pipe, char& c)
{
  boost::system::error_code ec;

  ba::read(pipe, ba::buffer(&c, 1), ec);

  if (ec == ba::error::eof)
  {
    return false;
  }

  if (ec)
  {
    log::debug("async_system") << "read error:" << ec.message();
    return false;
  }

  return true;
}
}; // namespace wex::factory

wex::factory::process_imp::process_imp()
  : m_io(std::make_shared<ba::io_context>())
  , m_queue(std::make_shared<std::queue<std::string>>())
  , m_ep(*m_io)
  , m_op(*m_io)
  , m_ip(*m_io)
{
}

void wex::factory::process_imp::async_system(process* p)
{
  m_debug.store(p->m_eh_debug != nullptr);

  try
  {
    boost_async_system(p);

    thread_input(p);
    thread_output(p);
    thread_error(p);
  }
  catch (std::exception& e)
  {
    log("async_system") << e.what();
  }
}

void wex::factory::process_imp::boost_async_system(process* p)
{
  if (!p->data().start_dir().empty())
  {
    m_process = std::make_unique<boost::process::process>(
      *m_io,
      p->data().exe_path(),
      p->data().args(),
      bp::process_stdio{m_op, m_ip, m_ep},
      bp::process_start_dir{p->data().start_dir()});
  }
  else
  {
    m_process = std::make_unique<boost::process::process>(
      *m_io,
      p->data().exe_path(),
      p->data().args(),
      bp::process_stdio{m_op, m_ip, m_ep});
  }

  m_process->async_wait(
    [this, p](boost::system::error_code ec, int exit_code)
    {
      if (ec.value() != 0 && p->m_eh_out != nullptr)
      {
        WEX_POST(ID_SHELL_APPEND_ERROR, ec.message(), p->m_eh_out)
      }

      log::debug("async_system") << "exit" << p->data().exe();

      if (m_debug.load())
      {
        WEX_POST(ID_DEBUG_EXIT, "", p->m_eh_debug)
      }
    });

  log::debug("async_system")
    << p->data().exe() << "wd:" << p->data().start_dir();

  WEX_POST(ID_SHELL_APPEND_START, "", p->m_eh_out)
  WEX_POST(ID_SHELL_APPEND, p->data().exe() + "\n", p->m_eh_out)
}

bool wex::factory::process_imp::is_running() const
{
  return m_process != nullptr && m_process->running();
}

bool wex::factory::process_imp::stop(wxEvtHandler* e)
{
  if (is_running())
  {
    m_io->stop();

    if (m_debug.load() && e != nullptr)
    {
      WEX_POST(ID_DEBUG_EXIT, "", e)
    }

    m_process->terminate();

    return true;
  }

  return false;
}

void wex::factory::process_imp::thread_error(const process* p)
{
  std::thread v(
    [debug = m_debug.load(), &dbg = p->m_eh_debug, out = p->m_eh_out, this]
    {
      std::string text;
      char        c;

      while (read_from_pipe(m_ep, c))
      {
        text.push_back(c);

        if (c == '\n')
        {
          WEX_POST(ID_SHELL_APPEND_ERROR, text, out)

          if (debug)
          {
            WEX_POST(ID_DEBUG_STDOUT, text, dbg)
          }

          text.clear();
        }
      }
    });

  v.detach();
}

void wex::factory::process_imp::thread_input(const process* p)
{
  std::thread t(
    [debug = m_debug.load(), &dbg = p->m_eh_debug, &out = p->m_eh_out, this]
    {
      std::string text, line;
      line.reserve(600);
      text.reserve(600);
      char c;

      while (read_from_pipe(m_ip, c))
      {
        text.push_back(c);

        if (debug)
        {
          line.push_back(c);

          if (c == '\n')
          {
            WEX_POST(ID_DEBUG_STDOUT, line, dbg)
            line.clear();
          }
        }

        if (text.size() > 500)
        {
          text += "...\n";
          WEX_POST(ID_SHELL_APPEND, text, out)
          std::string ignore;
          ba::read_until(m_ip, boost::asio::dynamic_buffer(ignore), "\n");
          text.clear();
        }
        else if (std::isspace(static_cast<unsigned char>(c)))
        {
          WEX_POST(ID_SHELL_APPEND, text, out)
          text.clear();
        }
      }

      WEX_POST(ID_SHELL_APPEND_FINISH, "", out)
    });

  t.detach();
}

void wex::factory::process_imp::thread_output(const process* p)
{
  std::thread u(
    [debug = m_debug.load(),
     io    = m_io,
     dbg   = p->m_eh_debug,
     queue = m_queue,
     this]
    {
      while (!io->stopped())
      {
        io->run_one_for(std::chrono::milliseconds(10));

        if (!queue->empty())
        {
          if (const auto& text(queue->front()); !io->stopped())
          {
            log::debug("async_system") << "write:" << text;

            ba::write(m_op, ba::buffer(text + "\n"), ba::transfer_all());

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
  if (text.empty() || !is_running())
  {
    return false;
  }

  m_queue->push(text);

  return true;
}
