////////////////////////////////////////////////////////////////////////////////
// Name:      data/cmdline.h
// Purpose:   Declaration of wex::data::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
namespace data
{
/// This class offers data for the command line parser class.
class cmdline
{
public:
  /// Constructor from commandline text.
  cmdline(const std::string& text)
    : m_string(text)
  {
    ;
  };

  /// Constructor from commandline argument count and vector.
  cmdline(int ac, char* av[])
    : m_ac(ac)
    , m_av(av)
  {
    ;
  };

  /// Returns argument count.
  int ac() const { return m_ac; }

  /// Returns arguments.
  char** av() const { return m_av; }

  /// Returns help.
  const auto& help() const { return m_help; }

  /// Sets help.
  cmdline& help(const std::string& rhs)
  {
    m_help = rhs;
    return *this;
  };

  /// Returns save.
  bool save() const { return m_save; }

  /// Sets save.
  cmdline& save(bool rhs)
  {
    m_save = rhs;
    return *this;
  };

  /// Returns command line string.
  const auto& string() const { return m_string; }

  /// Sets command line string.
  cmdline& string(const std::string& rhs)
  {
    m_string = rhs;
    return *this;
  };

private:
  const int m_ac{0};
  char**    m_av{nullptr};
  bool      m_save{false};

  std::string m_help, m_string;
};
}; // namespace data
}; // namespace wex
