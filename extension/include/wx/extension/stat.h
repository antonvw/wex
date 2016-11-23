////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sys/stat.h> // for stat
#include <wx/datetime.h>
#include <wx/filefn.h>

/// Adds IsOk to the stat base class, several methods
/// to get/update on the stat members, and Sync to sync
/// the stat from disk.
class wxExStat : public stat
{
public:
  /// Default constructor. Calls sync.
  wxExStat(const std::string& fullpath = std::string()){
    Sync(fullpath);};

  /// Returns the modification time.
  /// From wxFileName class GetModificationTime is available as well,
  /// this one returns string and only uses the stat member, and is fast.
  const std::string GetModificationTime() const {
    return wxDateTime(st_mtime).FormatISOCombined(' ').ToStdString();};

  /// Returns true if the stat is okay (last sync was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const {
#ifdef _MSC_VER
    return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));};
#else
    return (m_IsOk && access(m_FullPath.c_str(), W_OK) == -1);};
#endif

  /// Sets (syncs) this stat, returns result and keeps it in IsOk.
  bool Sync() {
    if (m_FullPath.empty())
    {
      m_IsOk = false;
    }
    else
    {
#ifdef _MSC_VER
      m_IsOk = (stat(m_FullPath.c_str(), this) != -1);
#else
      m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
#endif
    }
    return m_IsOk;};

  /// Sets the fullpath member, then syncs.
  bool Sync(const std::string& fullpath) {
    m_FullPath = fullpath;
    return Sync();}
private:
  std::string m_FullPath;
  bool m_IsOk;
};
