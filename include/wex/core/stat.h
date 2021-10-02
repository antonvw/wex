////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wex::file_stat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sys/stat.h>
// import gives error
#include <string>

namespace wex
{
/// Adds several methods to get/update the file status,
/// and sync to sync the status from disk.
class file_stat : private stat
{
public:
  /// See also chrono, uses same format.
  static inline const std::string TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

  /// Default constructor. Calls sync.
  explicit file_stat(const std::string& path = std::string()) { sync(path); }

  /// Returns the creation time.
  const std::string get_creation_time(
    /// the format as used by std::put_time
    const std::string& format = TIME_FORMAT) const;

  /// Returns the modification time.
  const std::string get_modification_time(
    /// the format as used by std::put_time
    const std::string& format = TIME_FORMAT) const;

  /// Returns atime.
  auto get_st_atime() const { return st_atime; }

  /// Returns ctime.
  auto get_st_ctime() const { return st_ctime; }

  /// Returns mtime.
  auto get_st_mtime() const { return st_mtime; }

  /// Returns size.
  auto get_st_size() const { return st_size; }

  /// Returns true if the stat is okay (last sync was okay).
  bool is_ok() const { return m_is_ok; }

  /// Returns true if this stat is readonly.
  bool is_readonly() const;

  /// Returns path.
  const auto& path() const { return m_fullpath; }

  /// Sets (syncs) this stat, returns result and keeps it in is_ok.
  bool sync();

  /// Sets the path member, then syncs.
  bool sync(const std::string& path)
  {
    m_fullpath = path;
    return sync();
  }

private:
  std::string m_fullpath;
  bool        m_is_ok;
};
} // namespace wex