////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wex::factory::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <future>

#include <wex/core/log.h>
#include <wex/factory/process.h>

#include "data-to-std-in.h"
#include "process-imp.h"

wex::factory::process::process()
  : m_imp(std::make_shared<process_imp>())
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

bool wex::factory::process::async_system(const wex::process_data& data)
{
  if (m_eh_out != nullptr)
  {
    m_data = data;
    m_imp->async_system(this); // this is a void
    return true;
  }

  log::trace("async_system needs set_handler_out");

  return false;
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
    return m_imp->stop(m_eh_debug);
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
    std::future<std::string> of, ef;
    data_to_std_in           data_std_in(data);

    // clang-format off
    const int ec = bp::system(
      data.exe_path(),
      bp::args = data.args(),
      bp::start_dir = data.start_dir(),
      bp::std_in < data_std_in.std_in(),
      bp::std_out > of,
      bp::std_err > ef);
    // clang-format on

    if (of.valid())
    {
      m_stdout = of.get();
    }

    if (ef.valid())
    {
      m_stderr = ef.get();
    }

    if (data.std_in().empty())
    {
      boost::process::v1::std_in
        .close(); // e.g. for svn a password is required, not yet ok
      log::trace("closing stdin");
    }

    if (!ec)
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

  return 1;
}

bool wex::factory::process::write(const std::string& text)
{
  return m_imp->write(text);
}
