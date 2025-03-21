////////////////////////////////////////////////////////////////////////////////
// Name:      path.h
// Purpose:   Declaration of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <filesystem>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <wex/core/file-status.h>

namespace wex
{
class file;

/// Offers functionality to handle paths.
class path
{
  friend class file; // it might update file_status
public:
  /// Flags for path logging.
  /// Default only logs the filename (all flags are off).
  enum
  {
    LOG_MOD  = 0, ///< adds 'modified'
    LOG_SYNC = 1, ///< adds 'synchronized'
    LOG_PATH = 2  ///< uses 'path' instead of 'filename'
  };

  /// A typedef containing log flags.
  typedef std::bitset<3> log_t;

  // Static interface.

  /// Returns current path.
  static wex::path current();

  /// Sets current path.
  static void current(const wex::path& p);

  // Others.

  /// Default constructor taking a std::filesystem::path.
  /// If path is empty, it saves the current path,
  /// and when destructed restores it to current.
  path(
    /// the path
    const std::filesystem::path& p = std::filesystem::path(),
    /// the status, used for log
    log_t t = 0);

  /// Constructor using string path.
  explicit path(const std::string& path, log_t t = 0);

  /// Constructor using a char array.
  explicit path(const char* path, log_t t = 0);

  /// Constructor from a vector of paths.
  explicit path(const std::vector<std::string>& v, log_t t = 0);

  /// Constructor using a path and a name.
  path(const path& p, const std::string& name, log_t = 0);

  /// Copy constructor.
  path(const path& r);

  /// Destructor.
  ~path();

  /// Spaceship operator.
  auto operator<=>(const path& r) const { return data() <=> r.data(); }

  /// == Operator.
  bool operator==(const path& r) const { return data() == r.data(); }

  /// << Operator.
  friend std::ostream& operator<<(std::ostream& os, const path& rhs);

  /// Appends path.
  path& append(const path& path);

  /// Returns the internal path.
  // (cannot be auto)
  const std::filesystem::path& data() const { return m_path; }

  /// Returns true if the directory with this name exists.
  bool dir_exists() const;

  /// Returns true if path is empty.
  bool empty() const { return m_path.empty(); }

  /// Returns true if path corresponds to an existing file or directory.
  bool exists() const;

  /// Returns path extension component (including the .).
  const std::string extension() const { return m_path.extension().string(); }

  /// Returns true if the file with this name exists.
  bool file_exists() const;

  /// Returns path filename (including extension) component.
  const std::string filename() const { return m_path.filename().string(); }

  /// Returns true if this path is absolute.
  bool is_absolute() const { return m_path.is_absolute(); }

  /// Returns true if this path (stat) is readonly.
  bool is_readonly() const { return m_stat.is_readonly(); }

  /// Returns true if this path is relative.
  bool is_relative() const { return m_path.is_relative(); }

  /// Logs info about this class.
  std::stringstream log() const;

  /// Make this path absolute.
  path& make_absolute();

  /// Returns path name component.
  const std::string name() const { return m_path.stem().string(); }

  /// Opens this path using registered mime type.
  /// Returns false if no mime type is found.
  bool open_mime() const;

  /// Returns original path.
  const auto& original() { return m_path_original; }

  /// Returns path path component.
  const std::string parent_path() const
  {
    return m_path.parent_path().string();
  }

  /// Returns path components.
  const std::vector<path> paths() const;

  /// Replaces filename.
  path& replace_filename(const std::string& filename);

  /// Sets new log type.
  void set_log(const log_t& t) { m_log = t; }

  /// Returns the stat.
  const auto& stat() const { return m_stat; }

  /// Returns the path as a string.
  const auto string() const { return m_path.string(); }

private:
  std::filesystem::path m_path, m_path_original;

  file_status m_stat;
  log_t       m_log{0};
};
}; // namespace wex
