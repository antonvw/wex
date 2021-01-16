////////////////////////////////////////////////////////////////////////////////
// Name:      chrono.cpp
// Purpose:   Implementation of wex::chrono class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iomanip>
#include <sstream>
#include <wex/chrono.h>
#include <wex/log.h>

const std::string wex::now(const std::string& format)
{
  return chrono(format).get_time(std::time(nullptr));
}

wex::chrono::chrono(const std::string& format)
  : m_format(format)
{
}

std::string wex::chrono::get_time(time_t tt) const
{
  std::stringstream ss;

  ss << std::put_time(std::localtime(&tt), m_format.c_str());

  return ss.str();
}

std::tuple<bool, time_t> wex::chrono::get_time(const std::string& text) const
{
  std::tm           tm = {0};
  std::stringstream ss(text);

  ss >> std::get_time(&tm, m_format.c_str());

  if (ss.fail())
  {
    wex::log("get_time") << ss << "format:" << m_format;
    return {false, 0};
  }

  const time_t t(mktime(&tm));

  return {t != -1, t};
}
