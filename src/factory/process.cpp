////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/core/log.h>
#include <wex/factory/process.h>

#include <boost/process/start_dir.hpp>
#include <boost/process/stdio.hpp>
#include <boost/system/error_code.hpp>

#include "data-to-std-in.h"
#include "process-imp.h"

wex::factory::process::process() = default;

wex::factory::process::~process()
{
  stop();
}

void wex::factory::process::async_sleep_for(const std::chrono::milliseconds& ms)
{
  std::this_thread::sleep_for(ms);
}

bool wex::factory::process::async_system(const wex::process_data& data)
{
  if (m_eh_out != nullptr)
  {
    m_imp  = std::make_shared<process_imp>();
    m_data = data;
    m_imp->async_system(this); // this is a void
    return true;
  }

  log::trace("async_system needs set_handler_out");

  return false;
}

bool wex::factory::process::is_debug() const
{
  return m_imp != nullptr && m_imp->is_debug();
}

bool wex::factory::process::is_running() const
{
  return m_imp != nullptr && m_imp->is_running();
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
    return m_imp != nullptr && m_imp->stop(m_eh_debug);
  }
  catch (std::exception& ex)
  {
    log(ex) << "stop" << m_data.exe();
  }

  return false;
}

int wex::factory::process::system(const wex::process_data& data)
{
  try
  {
    ba::io_context    ctx;
    ba::readable_pipe op{ctx}, ep{ctx};
    data_to_std_in    data_std_in(data);

    bp::process c{
      ctx,
      data.exe_path(),
      data.args(),
      bp::process_stdio{data_std_in.std_in(), op, ep}};
//      bp::process_start_dir{data.start_dir()}};

    boost::system::error_code bec;

    ba::read(op, ba::dynamic_buffer(m_stdout), bec);
    ba::read(ep, ba::dynamic_buffer(m_stderr), bec);

    c.wait();

    const int ec = c.exit_code();

    if (ec >= 0)
    {
      log::debug("system") << data.log();
    }
    else
    {
      const auto& text(!m_stderr.empty() ? ":" + m_stderr : std::string());
      log("system") << data.log() << "ec:" << ec << text
                    << "wd:" << data.start_dir();
      log::status("system") << text << data.log();
    }

    m_data = data;

    return ec;
  }
  catch (std::exception& e)
  {
    log(e) << data.log();

    m_stdout.clear();
    m_stderr = e.what();
  }
  catch (...)
  {
    log("system unknown exception") << data.log();
  }

  return -1;
}

bool wex::factory::process::write(const std::string& text)
{
  return m_imp != nullptr && m_imp->write(text);
}
