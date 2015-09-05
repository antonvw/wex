////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sys/stat.h> // for stat
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/string.h>

/// Adds IsOk to the stat base class, several methods
/// to get/update on the stat members, and Sync to sync
/// the stat from disk.
class WXDLLIMPEXP_BASE wxExStat : public stat
{
public:
  /// Default constructor. Calls sync.
  wxExStat(const wxString& fullpath = wxEmptyString){
    Sync(fullpath);};

  /// Gets the modification time.
  /// From wxFileName class GetModificationTime is available as well,
  /// this one returns string and only uses the stat member, and is fast.
  const wxString GetModificationTime() const {
    return wxDateTime(st_mtime).FormatISOCombined(' ');};

  /// Returns true if the stat is okay (last sync was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const {
    return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));};

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
  bool Sync(const wxString& fullpath) {
    m_FullPath = fullpath;
    return Sync();}
private:
  wxString m_FullPath;
  bool m_IsOk;
};
