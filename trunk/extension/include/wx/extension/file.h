////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class 'wxExFile'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFILE_H
#define _EXFILE_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/extension/filename.h>
#include <wx/extension/stat.h>

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
protected:
  /// Invoked by FileLoad, allows you to load the file.
  virtual void DoFileLoad(bool synced = false) = 0;

  /// Invoked by FileSave, allows you to save the file.
  virtual void DoFileSave(bool save_as = false) = 0;
private:
  void Assign(const wxFileName& filename);
  /// Called if file needs to be synced.
  void FileSync();
  // Take care that filename and stat are in sync.
  bool MakeAbsolute();

  wxExFileName m_FileName;
  wxExStat m_Stat;
};
#endif
