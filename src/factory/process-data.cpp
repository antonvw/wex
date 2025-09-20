////////////////////////////////////////////////////////////////////////////////
// Name:      process-data.cpp
// Purpose:   Implementation of class wex::factory::process_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <numeric>

#include <boost/process/v2/environment.hpp>

#include <wex/common/tostring.h>
#include <wex/core/core.h>
#include <wex/core/path.h>
#include <wex/factory/process-data.h>

namespace bp = boost::process::v2;

wex::process_data::process_data(const std::string& exe, const std::string& args)
  : m_exe(exe)
  , m_args(args)
{
}

const std::vector<std::string> wex::process_data::args() const
{
  if (!m_args.empty())
  {
    return to_vector_string(m_args).get();
  }

  const auto pos = m_exe.find(' ');

  if (pos == std::string::npos)
  {
    return std::vector<std::string>{};
  }

  return to_vector_string(m_exe.substr(pos + 1)).get();
}

wex::process_data& wex::process_data::args(const std::string& rhs)
{
  m_args = rhs;
  return *this;
}

wex::process_data& wex::process_data::exe(const std::string& rhs)
{
  m_exe = rhs;
  return *this;
}

const std::string wex::process_data::exe_path() const
{
  const auto& p(path(find_before(m_exe, " ")));

  if (p.string().empty())
  {
    throw std::invalid_argument("Cannot execute empty string");
  }
  if (!p.file_exists())
  {
    if (const auto& bop(bp::environment::find_executable(p.string()));
        !bop.empty())
    {
      return bop.string();
    }

    throw std::invalid_argument("Could not find: " + p.string());
  }

  return p.string();
}

const std::string wex::process_data::log() const
{
  const auto& arg_v(args());
  std::string exe("exe: ");

  try
  {
    exe += exe_path();
  }
  catch (std::exception& e)
  {
    exe += e.what();
  }

  return exe +
         (!arg_v.empty() ?
            " args:" + std::ranges::fold_left(
                         arg_v,
                         std::string(""),
                         [](const std::string& a, const std::string& b)
                         {
                           return a + " " + b;
                         }) :
            std::string()) +
         (!m_start_dir.empty() ? " dir: " + m_start_dir : std::string()) +
         (!m_stdin.empty() ? " stdin: " + m_stdin : std::string());
}

wex::process_data& wex::process_data::start_dir(const std::string& rhs)
{
  m_start_dir = rhs;
  return *this;
}

wex::process_data& wex::process_data::std_in(const std::string& rhs)
{
  m_stdin = rhs;
  return *this;
}
