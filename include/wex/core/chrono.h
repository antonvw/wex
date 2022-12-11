////////////////////////////////////////////////////////////////////////////////
// Name:      chrono.h
// Purpose:   Declaration of wex::chrono class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <string>
#include <tuple>

namespace wex
{
/// Offers a chrono class.
class chrono
{
public:
  /// The default time format.
  static inline const std::string TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

  /// Precision used for outputting time.
  enum precision_t
  {
    PRECISION_SEC,   ///< seconds
    PRECISION_MILLI, ///< millisecond
    PRECISION_MICRO, ///< microsecond
    PRECISION_NANO,  ///< nanosecond
  };

  /// Default constructor.
  chrono(
    const std::string& format    = TIME_FORMAT,
    precision_t        precision = PRECISION_SEC);

  /// Returns time string for a time_t. Precision is not used.
  std::string get_time(time_t tt) const;

  /// Returns time string for a timespec, using precision.
  std::string get_time(const timespec& ts) const;

  /// Returns time string for a time_point, using precision.
  std::string
  get_time(const std::chrono::time_point<std::chrono::system_clock>& tp) const;

  /// Returns time_t for a time string.
  std::tuple<
    /// true if time string could be converted into time_t
    bool,
    /// the converted time
    time_t>
  get_time(const std::string& time) const;

private:
  const std::string m_format;
  const precision_t m_precision;
};

/// Returns now as a string.
const std::string now(
  const std::string&  format    = chrono::TIME_FORMAT,
  chrono::precision_t precision = chrono::PRECISION_SEC);
} // namespace wex
