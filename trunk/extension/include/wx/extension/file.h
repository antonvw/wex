/******************************************************************************\
* File:          file.h
* Purpose:       Declaration of wxWidgets file extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXFILE_H
#define _EXFILE_H

#include <sys/stat.h> // for stat
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/extension/lexer.h>

/// Adds IsOk to the stat base class, and several methods
/// to get/update on the stat members.
class wxExStat : public stat
{
public:
  /// Default constructor. Calls Sync.
  wxExStat(const wxString& fullpath = wxEmptyString) {
    Sync(fullpath);}

  /// Gets fullpath member.
  const wxString& GetFullPath() const {return m_FullPath;};

  /// Gets the modification time.
  /// From wxFileName class GetModificationTime is available as well,
  /// this one returns string and only uses the stat member, and is fast.
  const wxString GetModificationTime(
    const wxString& format = wxDefaultDateTimeFormat) const;

  /// Returns true if the stat is okay (last update was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const {
    return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));};

  /// Updates this stat, returns result and keeps it in IsOk.
  bool Sync();

  /// Updates fullpath member, then Syncs.
  bool Sync(const wxString& fullpath);
private:
  wxString m_FullPath;
  bool m_IsOk;
};

/// Adds an wxExStat and an wxExLexer member to wxFileName.
class wxExFileName : public wxFileName
{
public:
  /// Default constructor.
  wxExFileName(
    const wxString& fullpath = wxEmptyString,
    wxPathFormat format = wxPATH_NATIVE)
    : wxFileName(fullpath, format)
    , m_Stat(fullpath) {
    SetLexer();}

  /// Copy constructor from a wxFileName.
  wxExFileName(const wxFileName& filename)
    : wxFileName(filename)
    , m_Stat(filename.GetFullPath()) {
    SetLexer();}

  /// Assignment operator.
  wxExFileName& operator=(const wxExFileName& f)
  {
    m_Lexer = f.m_Lexer;
    m_Stat = f.m_Stat;
    Assign(f.GetFullPath());
    return *this;
  };

  /// Gets the icon index for this filename (uses the file extension to get it).
  int GetIconID() const;

  /// Gets the lexer.
  const wxExLexer& GetLexer() const {return m_Lexer;};

  /// Gets the stat.
  const wxExStat& GetStat() const {return m_Stat;};

  /// Gets the stat (allowing you to call Sync for it).
  wxExStat& GetStat() {return m_Stat;};

  /// If specified lexer is empty, use one of the lexers from config
  /// according to match on the file fullname.
  /// Otherwise use specified lexer.
  /// Text is used if lexer is empty, to override settings from config
  /// if no match was found, to match special cases.
  void SetLexer(
    const wxString& lexer = wxEmptyString,
    const wxString& text = wxEmptyString);
private:
  wxExLexer m_Lexer;
  wxExStat m_Stat;
};

/// Adds an wxExFileName, an wxExStat member that can be used for synchronization,
/// and several File* methods to wxFile. All the File* methods update
/// the wxExStat member.
class wxExFile : public wxFile
{
public:
  /// Default constructor.
  wxExFile();

  /// Opens a file with a filename.
  wxExFile(const wxString& filename, wxFile::OpenMode mode = wxFile::read);

  /// Destructor.
  /// NB: for wxFile the destructor is not virtual so you should not use wxFile polymorphically.
  /// So do it here.
  virtual ~wxExFile() {;};

  /// Sets the filename member.
  virtual bool FileNew(const wxExFileName& filename = wxExFileName());

  /// Sets the filename member and opens the file.
  virtual bool FileOpen(const wxExFileName& filename);

  /// Invoked by FileSaveAs, allows you to save your file.
  /// The default closes the file.
  virtual bool FileSave();

  /// Saves under different name.
  bool FileSaveAs(const wxString filename);

  /// Called if file needs to be synced.
  /// The default calls FileOpen, and updates status text.
  virtual bool FileSync();

  /// Returns whether contents have been changed.
  /// Default returns false.
  virtual bool GetContentsChanged() {return false;};

  /// Reset contents changed.
  /// Default does nothing.
  virtual void ResetContentsChanged() {;};

  /// Invokes FileSync if this file needs to be synced.
  /// Returns false if no check was done (e.g. this file was opened).
  bool CheckFileSync();

  /// Gets the file name.
  const wxExFileName& GetFileName() const {return m_FileName;}

  /// Gets the stat.
  /// By comparing this with the stat from GetFileName
  /// you can detect whether the file needs to be synced.
  const wxExStat& GetStat() const {return m_Stat;};

  /// Gets the wildcard.
  const wxString& GetWildcard() const {return m_Wildcard;}

  /// Reads this file into a buffer.
  const wxCharBuffer Read(wxFileOffset seek_position = 0);

  /// Sets the wild card member.
  void SetWildcard(const wxString& wildcard) {m_Wildcard = wildcard;};
protected:
  wxExFileName m_FileName; ///< the filename
private:
  // Take care that filename and stat are in sync.
  bool MakeAbsolute();

  wxExStat m_Stat;
  wxString m_Wildcard;
};
#endif
