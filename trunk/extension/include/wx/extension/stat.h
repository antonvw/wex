////////////////////////////////////////////////////////////////////////////////
// Name:      stat.h
// Purpose:   Declaration of class 'wxExStat'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTAT_H
#define _EXSTAT_H

#include <sys/stat.h> // for stat

/// Adds IsOk to the stat base class, and several methods
/// to get/update on the stat members.
class wxExStat : public stat
{
public:
  /// Default constructor. Calls Sync.
  wxExStat(const wxString& fullpath = wxEmptyString) {
    Sync(fullpath);}

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

  /// Sets this stat, returns result and keeps it in IsOk.
  bool Sync() {
#ifdef __UNIX__
    m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
#else
    m_IsOk = (stat(m_FullPath.c_str(), this) != -1);
#endif
    return m_IsOk;};

  /// Sets the fullpath member, then Syncs.
  bool Sync(const wxString& fullpath) {
    m_FullPath = fullpath;
    return Sync();};
private:
  wxString m_FullPath;
  bool m_IsOk;
};
#endif
