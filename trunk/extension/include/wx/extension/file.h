/******************************************************************************\
* File:          file.h
* Purpose:       Declaration of wxExtension file classes
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
  const wxString GetModificationTime() const;

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

/// Adds several File* methods to wxFile. All the File* methods update
/// the wxExStat member. Also takes care of synchronization,
/// all you have to do is call CheckFileSync once in a while.
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

  /// Invokes DoFileLoad if this file needs to be synced.
  /// Returns false if no check was done (e.g. this file was opened).
  bool CheckFileSync();

  /// Sets the filename member and invokes DoFileLoad().
  bool FileLoad(const wxExFileName& filename);

  /// Sets the filename member.
  void FileNew(const wxExFileName& filename = wxExFileName());

  /// Sets the filename member and invokes DoFileSave().
  bool FileSave(const wxString filename = wxEmptyString);

  /// Returns whether contents have been changed.
  virtual bool GetContentsChanged() const = 0;

  /// Gets the file name.
  const wxExFileName& GetFileName() const {return m_FileName;}

  /// Gets the stat.
  const wxExStat& GetStat() const {return m_Stat;};

  /// Reads this file into a buffer.
  const wxCharBuffer Read(wxFileOffset seek_position = 0);

  /// Reset contents changed.
  virtual void ResetContentsChanged() = 0;

  /// Sets specified lexer for filename.
  void SetFileNameLexer(
    const wxString& lexer = wxEmptyString,
    const wxString& text = wxEmptyString) {
      m_FileName.SetLexer(lexer, text);};
protected:
  /// Invoked by FileLoad, allows you to load the file.
  virtual void DoFileLoad(bool synced = false) = 0;

  /// Invoked by FileSave, allows you to save the file.
  virtual void DoFileSave(bool save_as = false) = 0;
private:
  /// Called if file needs to be synced.
  void FileSync();

  // Take care that filename and stat are in sync.
  bool MakeAbsolute();

  wxExFileName m_FileName;
  wxExStat m_Stat;
};
#endif
