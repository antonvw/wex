////////////////////////////////////////////////////////////////////////////////
// Name:      file-status.h
// Purpose:   Declaration of wex::file_status class
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
class file_status
{
public:
  /// See also chrono, uses same format.
  static inline const std::string TIME_FORMAT = "%Y-%m-%d %H:%M:%S";

  /// Default constructor. Calls sync.
  explicit file_status(const std::string& path = std::string()) { sync(path); }

  /// Returns access time.
  time_t get_access_time() const;

  /// Returns creation ctime.
  time_t get_creation_time() const;

  /// Returns the creation time as a string.
  const std::string get_creation_time_str(
    /// the format as used by std::put_time
    const std::string& format = TIME_FORMAT) const;

  /// Returns modification time.
  time_t get_modification_time() const;

  /// Returns the modification time as a string.
  const std::string get_modification_time_str(
    /// the format as used by std::put_time
    const std::string& format = TIME_FORMAT) const;

  /// Returns size.
  off_t get_size() const;

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
  struct stat m_file_status;
};
} // namespace wex
