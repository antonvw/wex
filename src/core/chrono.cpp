////////////////////////////////////////////////////////////////////////////////
// Name:      chrono.cpp
// Purpose:   Implementation of wex::chrono class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <wex/core/chrono.h>

#include <iomanip>
#include <sstream>

using namespace ::std::chrono;

constexpr timespec
time_point_to_timespec(time_point<system_clock, nanoseconds> tp)
{
  auto secs = time_point_cast<seconds>(tp);
  auto ns =
    time_point_cast<nanoseconds>(tp) - time_point_cast<nanoseconds>(secs);

  return timespec{
    secs.time_since_epoch().count(),
    static_cast<long>(ns.count())};
}

constexpr nanoseconds timespec_to_duration(timespec ts)
{
  auto duration = seconds{ts.tv_sec} + nanoseconds{ts.tv_nsec};

  return duration_cast<nanoseconds>(duration);
}

constexpr time_point<system_clock, nanoseconds>
timespec_to_time_point(const timespec& ts)
{
  return time_point<system_clock, nanoseconds>{
    duration_cast<system_clock::duration>(timespec_to_duration(ts))};
}

const std::string
wex::now(const std::string& format, chrono::precision_t precision)
{
  return chrono(format, precision).get_time(std::chrono::system_clock::now());
}

wex::chrono::chrono(const std::string& format, precision_t precision)
  : m_format(format)
  , m_precision(precision)
{
}

std::string wex::chrono::get_time(time_t tt) const
{
  std::stringstream ss;

  ss << std::put_time(std::localtime(&tt), m_format.c_str());

  return ss.str();
}

std::string wex::chrono::get_time(const timespec& ts) const
{
  std::stringstream ss;

  ss << get_time(ts.tv_sec);

  if (m_precision != precision_t::SEC)
  {
    switch (const auto tp = timespec_to_time_point(ts); m_precision)
    {
      case precision_t::MICRO:
      {
        const auto tp_ms =
          std::chrono::duration_cast<std::chrono::microseconds>(
            tp.time_since_epoch()) %
          1000000;
        ss << '.' << std::setfill('0') << std::setw(6) << tp_ms.count();
      }
      break;

      case precision_t::MILLI:
      {
        const auto tp_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(
            tp.time_since_epoch()) %
          1000;
        ss << '.' << std::setfill('0') << std::setw(3) << tp_ms.count();
      }
      break;

      case precision_t::NANO:
      {
        const auto tp_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             tp.time_since_epoch()) %
                           1000000000;
        ss << '.' << std::setfill('0') << std::setw(9) << tp_ms.count();
      }
      break;

      default:
        assert(0);
    }
  }

  return ss.str();
}

std::string wex::chrono::get_time(
  const std::chrono::time_point<std::chrono::system_clock>& tp) const
{
  return get_time(time_point_to_timespec(tp));
}

std::optional<time_t> wex::chrono::get_time(const std::string& text) const
{
  std::tm           tm = {0};
  std::stringstream ss(text);

  ss >> std::get_time(&tm, m_format.c_str());

  if (ss.fail())
  {
    return {};
  }

  const time_t t(mktime(&tm));

  return t != -1 ? std::optional<time_t>{t} : std::nullopt;
}
