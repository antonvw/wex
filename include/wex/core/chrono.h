////////////////////////////////////////////////////////////////////////////////
// Name:      chrono.h
// Purpose:   Declaration of wex::chrono class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace wex
{
/// Offers a chrono class.
class chrono
{
public:
  /// The default time format.
  static inline const std::string TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

  /// Precision used for outputting time.
  enum class precision_t
  {
    SEC,   ///< seconds
    MILLI, ///< millisecond
    MICRO, ///< microsecond
    NANO,  ///< nanosecond
  };

  /// Default constructor.
  chrono(
    const std::string& format    = TIME_FORMAT,
    precision_t        precision = precision_t::SEC);

  /// Returns time string for a time_t. Precision is not used.
  std::string get_time(time_t tt) const;

  /// Returns time string for a timespec, using precision.
  std::string get_time(const timespec& ts) const;

  /// Returns time string for a time_point, using precision.
  std::string
  get_time(const std::chrono::time_point<std::chrono::system_clock>& tp) const;

  /// Returns time_t for a time string.
  std::optional<time_t> get_time(const std::string& time) const;

private:
  const std::string m_format;
  const precision_t m_precision;
};

/// Returns now as a string.
const std::string now(
  const std::string&  format    = chrono::TIME_FORMAT,
  chrono::precision_t precision = chrono::precision_t::SEC);
} // namespace wex
