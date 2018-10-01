////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class wxExFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <istream>
#include <memory>
#include <wx/extension/path.h>

class wxExFileImp;

/// Offers several methods to read / write files. All the File* methods update
/// the wxExStat member. Also takes care of synchronization,
/// all you have to do is call CheckSync once in a while.
class wxExFile
{
public:
  /// Default constructor.
  /// The open_file parameter specifies the behaviour of FileLoad and
  /// FileSave, if true, the file is opened before calling DoFileLoad
  /// and DoFileSave, if false the file is not opened.
  wxExFile(bool open_file = true);

  /// Constructor taking a path, and opens the file.
  wxExFile(
    const wxExPath& p,
    std::ios_base::openmode mode = 
      std::ios_base::in | std::ios_base::out,
    bool open_file = true);
  
  /// Constructor taking a filename, and opens the file.
  wxExFile(
    const std::string& filename,
    std::ios_base::openmode mode =
      std::ios_base::in | std::ios_base::out,
    bool open_file = true);
  
  /// Copy constructor.
  wxExFile(const wxExFile& rhs);
  
  /// Destructor.
  virtual ~wxExFile();

  /// Assignment operator.
  wxExFile& operator=(const wxExFile& f);

  /// Checks whether this file can be synced, and 
  /// syncs (invokes DoFileLoad) the file if so.
  /// Returns true if this file was synced.
  bool CheckSync();

  /// Sets the path, opens the file if asked for,
  /// invokes DoFileLoad, and closes the file again.
  bool FileLoad(const wxExPath& p);

  /// Sets the path and invokes DoFileNew.
  bool FileNew(const wxExPath& p);

  /// Sets the path if path is ok, opens the file if asked for,
  /// invokes DoFileSave, and closes the file again.
  bool FileSave(const wxExPath& p = wxExPath());

  /// Returns the path.
  const wxExPath& GetFileName() const;
  
  /// Returns true if file is opened.
  bool IsOpened() const;

  /// Opens current path.
  bool Open(
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

  /// Opens specified path.
  bool Open(
    const wxExPath& p, 
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

  /// Reads this file into a buffer.
  /// Return nullptr in case of error.
  const std::string* Read(std::streampos seek_position = 0);

  /// Writes file from buffer.
  bool Write(const char* s, size_t n);
  
  /// Writes file from string.
  bool Write(const std::string& s);
public:
  /// Returns whether contents have been changed.
  virtual bool GetContentsChanged() const {return false;};

  /// Resets contents changed.
  virtual void ResetContentsChanged() {;};
protected:
  /// Invoked by FileLoad, allows you to load the file.
  /// The file is already opened, so you can call Read.
  /// If synced is true, this call was a result of
  /// CheckSync and not of FileLoad.
  virtual bool DoFileLoad(bool synced = false) {return false;};

  /// Invoked by FileNew, allows you to make a new (empty) file.
  virtual void DoFileNew() {;};

  /// Invoked by FileSave, allows you to save the file.
  /// The file is already opened.
  virtual void DoFileSave(bool save_as = false) {;};
private:
  void Assign(const wxExPath& p);
  bool FileLoad(bool synced);
  
  bool m_IsLoaded {false}, m_OpenFile;
  
  std::unique_ptr<std::string> m_Buffer;
  std::unique_ptr<wxExFileImp> m_File;
};
