////////////////////////////////////////////////////////////////////////////////
// Name:      process-data.cpp
// Purpose:   Implementation of class wex::factory::process_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <numeric>

#define BOOST_ASIO_HAS_STD_INVOKE_RESULT ON
#include <boost/process.hpp>

#include <wex/common/tostring.h>
#include <wex/core/core.h>
#include <wex/core/path.h>
#include <wex/factory/process-data.h>

namespace bp = boost::process;

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
  else if (const auto pos = m_exe.find(" "); pos == std::string::npos)
  {
    return std::vector<std::string>{};
  }
  else
  {
    return to_vector_string(m_exe.substr(pos + 1)).get();
  }
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

  if (!p.file_exists())
  {
    if (const auto& bop(bp::search_path(p.string())); !bop.empty())
    {
      return bop.string();
    }
  }

  return p.string();
}

const std::string wex::process_data::log() const
{
  const auto& arg_v(args());

  return "exe: " + exe_path() +
         (!arg_v.empty() ?
            " args:" + std::accumulate(
                         arg_v.begin(),
                         arg_v.end(),
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
