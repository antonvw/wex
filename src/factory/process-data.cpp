////////////////////////////////////////////////////////////////////////////////
// Name:      process-data.cpp
// Purpose:   Implementation of class wex::factory::process_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>

#include <wex/factory/process-data.h>

wex::process_data::process_data(const std::string& exe)
  : m_exe(exe)
{
}

const std::string wex::process_data::log() const
{
  return "exe: " + m_exe +
         (!m_start_dir.empty() ? " dir: " + m_start_dir : std::string()) +
         (!m_stdin.empty() ? " stdin: " + m_stdin : std::string());
}

wex::process_data& wex::process_data::exe(const std::string& rhs)
{
  m_exe = rhs;
  return *this;
}

wex::process_data& wex::process_data::start_dir(const std::string& rhs)
{
  m_start_dir = rhs;
  return *this;
}

wex::process_data& wex::process_data::stdin(const std::string& rhs)
{
  m_stdin = rhs;
  return *this;
}
