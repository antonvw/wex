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

#include <wx/file.h>
#include <wx/extension/filename.h>
#include <wx/extension/stat.h>

/// Adds several File* methods to wxFile. All the File* methods update
/// the wxExStat member. Also takes care of synchronization,
/// all you have to do is call CheckSync once in a while.
class wxExFile : public wxFile
{
public:
  /// Default constructor.
  wxExFile();

  /// Opens a file with a filename.
  wxExFile(const wxExFileName& filename, wxFile::OpenMode mode = wxFile::read);

  /// Destructor.
  /// NB: for wxFile the destructor is not virtual so 
  /// you should not use wxFile polymorphically.
  /// So do it here.
  virtual ~wxExFile() {;};

  /// Checks whether this file can be synced, and 
  /// syncs (invokes DoFileLoad) the file if so.
  /// Returns true if this file was synced.
  bool CheckSync();

  /// Sets the filename member, opens the file,
  /// invokes DoFileLoad, and closes the file again.
  bool FileLoad(const wxExFileName& filename);

  /// Sets the filename member and invokes DoFileNew.
  void FileNew(const wxExFileName& filename);

  /// Sets the filename member if filename is ok, opens the file,
  /// invokes DoFileSave, and closes the file again.
  bool FileSave(const wxExFileName& filename = wxExFileName());

  /// Returns whether contents have been changed.
  virtual bool GetContentsChanged() const {return false;};

  /// Gets the file name.
  const wxExFileName& GetFileName() const {return m_FileName;}

  /// Reads this file into a buffer.
  const wxCharBuffer Read(wxFileOffset seek_position = 0);

  /// Reset contents changed.
  virtual void ResetContentsChanged() {;};
protected:
  /// Invoked by FileLoad, allows you to load the file.
  /// The file is already opened, so you can call Read.
  /// If synced is true, this call was a result of
  /// CheckSync and not of FileLoad.
  virtual void DoFileLoad(bool synced = false) {;};

  /// Invoked by FileNew, allows you to make a new (empty) file.
  virtual void DoFileNew() {;};

  /// Invoked by FileSave, allows you to save the file.
  /// The file is already opened.
  virtual void DoFileSave(bool save_as = false) {;};
private:
  bool Assign(const wxExFileName& filename) {
    m_FileName = filename;
    m_Stat = filename.GetFullPath();
    return m_Stat.IsOk();};
  bool Get(bool synced);
  bool MakeAbsolute() {
    return 
      m_FileName.MakeAbsolute() &&
      m_FileName.m_Stat.Sync(m_FileName.GetFullPath()) &&
      m_Stat.Sync(m_FileName.GetFullPath());};

  wxExFileName m_FileName;
  wxExStat m_Stat; // used for syncing, no public access
};
#endif
