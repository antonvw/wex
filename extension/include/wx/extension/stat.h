////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTAT_H
#define _EXSTAT_H

#include <sys/stat.h> // for stat
#include <wx/string.h>

/// Adds IsOk to the stat base class, several methods
/// to get/update on the stat members, and Sync to sync
/// the stat from disk.
class WXDLLIMPEXP_BASE wxExStat : public stat
{
public:
  /// Default constructor. Calls Sync.
  wxExStat(const wxString& fullpath = wxEmptyString);

  /// Gets the fullpath member.
  const wxString& GetFullPath() const {return m_FullPath;};

  /// Gets the modification time.
  /// From wxFileName class GetModificationTime is available as well,
  /// this one returns string and only uses the stat member, and is fast.
  const wxString GetModificationTime() const {
    return wxDateTime(st_mtime).FormatISOCombined(' ');};

  /// Returns true if the stat is okay (last update was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const {
    return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));};

  /// Sets (syncs) this stat, returns result and keeps it in IsOk.
  bool Sync();

  /// Sets the fullpath member, then Syncs.
  bool Sync(const wxString& fullpath) {
    m_FullPath = fullpath;
    return Sync();}
private:
  wxString m_FullPath;
  bool m_IsOk;
};
#endif
