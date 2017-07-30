////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sys/stat.h> // for stat

/// Adds IsOk to the stat base class, several methods
/// to get/update on the stat members, and Sync to sync
/// the stat from disk.
class wxExStat : public stat
{
public:
  /// Default constructor. Calls sync.
  wxExStat(const std::string& fullpath = std::string()) {
    Sync(fullpath);};

  /// Returns the modification time.
  const std::string GetModificationTime() const;

  /// Returns true if the stat is okay (last sync was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const;

  /// Sets (syncs) this stat, returns result and keeps it in IsOk.
  bool Sync();

  /// Sets the fullpath member, then syncs.
  bool Sync(const std::string& fullpath) {
    m_FullPath = fullpath;
    return Sync();}
private:
  std::string m_FullPath;
  bool m_IsOk;
};
