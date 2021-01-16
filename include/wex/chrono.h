////////////////////////////////////////////////////////////////////////////////
// Name:      chrono.h
// Purpose:   Declaration of wex::chrono class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <tuple>

namespace wex
{
  /// Offers a chrono class.
  class chrono
  {
  public:
    static inline const std::string TIME_FORMAT = "%c";

    /// Default constructor.
    chrono(const std::string& format = chrono::TIME_FORMAT);

    /// Returns time string for a time_t.
    std::string get_time(time_t tt) const;

    /// Returns time_t for a time string.
    std::tuple<
      /// true if text could be converted into time_t
      bool,
      /// the converted time
      time_t>
    get_time(const std::string& time) const;

  private:
    const std::string m_format;
  };

  /// Returns now as a string.
  const std::string now(const std::string& format = chrono::TIME_FORMAT);
}; // namespace wex
