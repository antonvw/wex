////////////////////////////////////////////////////////////////////////////////
// Name:      temp-filename.h
// Purpose:   Declaration of class wex::temp_filename
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
/// Offers a class to generate temp filename, and access it's name.
/// If asked for, also removes this file.
class temp_filename
{
public:
  /// Default constructor, generates the temp filename.
  /// It just generates the filename, creating the file is not done.
  explicit temp_filename(bool cleanup = false);

  /// Destructor, removes the temp file if asked for.
  ~temp_filename();

  /// Returns the name (fullpath) of the temp file.
  const auto& name() const { return m_name; }

private:
  const bool        m_cleanup{false};
  const std::string m_name;
  static inline int m_no{0};
};
}; // namespace wex
