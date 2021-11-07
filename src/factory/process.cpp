////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/process.h>

#include "process-imp.h"

wex::factory::process::process()
  : m_imp(std::make_unique<process_imp>())
{
}

wex::factory::process::~process()
{
  stop();
}

void wex::factory::process::async_sleep_for(const std::chrono::milliseconds& ms)
{
  m_imp->async_sleep_for(ms);
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

const std::string wex::factory::process::get_exe() const
{
  return m_imp->exe();
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
  return m_imp->stop();
}

int wex::factory::process::system(
  const std::string& exe,
  const std::string& start_dir)
{
  const std::string log_dir(
    !start_dir.empty() ? "start_dir: " + start_dir : std::string());

  try
  {
    std::future<std::string> of, ef;

    const int ec = bp::system(
      bp::start_dir = start_dir,
      exe,
      bp::std_in<stdin, bp::std_out> of,
      bp::std_err > ef);

    if (of.valid())
      m_stdout = of.get();
    if (ef.valid())
      m_stderr = ef.get();

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
