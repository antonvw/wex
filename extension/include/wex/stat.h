////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wex::file_stat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sys/stat.h> // for stat

namespace wex
{
  /// Adds is_ok to the stat base class, several methods
  /// to get/update on the stat members, and sync to sync
  /// the stat from disk.
  class file_stat : public stat
  {
  public:
    static inline const std::string MOD_TIME_FORMAT = "%c";

    /// Default constructor. Calls sync.
    file_stat(const std::string& fullpath = std::string()) {
      sync(fullpath);};

    /// Returns the modification time.
    const std::string get_modification_time(
      /// the format as used by std::put_time
      const std::string& format = MOD_TIME_FORMAT) const;

    /// Returns true if the stat is okay (last sync was okay).
    bool is_ok() const {return m_is_ok;};

    /// Returns true if this stat is readonly.
    bool is_readonly() const;

    /// Sets (syncs) this stat, returns result and keeps it in is_ok.
    bool sync();

    /// Sets the fullpath member, then syncs.
    bool sync(const std::string& fullpath) {
      m_FullPath = fullpath;
      return sync();}
  private:
    std::string m_FullPath;
    bool m_is_ok;
  };
};
