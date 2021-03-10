////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wex::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wex/dir-data.h>
#include <wex/interruptible.h>
#include <wex/path.h>

namespace wex
{
  /// Offers find_files method.
  /// By overriding on_dir and on_file you can take care
  /// of what to do with the result.
  class dir : public interruptible
  {
  public:
    /// Constructor.
    dir(
      /// the dir to start finding
      const path& path,
      /// the dir data
      const data::dir& data = data::dir());

    /// Destructor.
    virtual ~dir() = default;

    /// Virtual interface.

    /// Do something with the dir.
    /// Not made pure virtual, to allow this
    /// class to be tested by calling find_files.
    virtual bool on_dir(const path&) { return true; };

    /// Do something with the file.
    /// Not made pure virtual, to allow this
    /// class to be tested by calling find_files.
    virtual bool on_file(const path&) { return true; };

    /// Other methods.

    /// Returns the data.
    const auto& data() const { return m_data; };

    /// Finds matching files.
    /// This results in recursive calls for on_dir and on_file.
    /// Returns number of files matching, or -1 if error.
    int find_files();

    /// Returns the path.
    const auto& get_path() const { return m_dir; };

  private:
    const path      m_dir;
    const data::dir m_data;
  };

  /// Returns all matching files into a vector of strings (without paths).
  std::vector<std::string> get_all_files(
    /// the dir to start finding
    const std::string& path,
    /// the dir data
    const data::dir& data = data::dir());
}; // namespace wex
