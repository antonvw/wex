////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class wxExFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <wx/file.h>
#include <wx/extension/path.h>
#include <wx/extension/stat.h>

/// Adds several File* methods to wxFile. All the File* methods update
/// the wxExStat member. Also takes care of synchronization,
/// all you have to do is call CheckSync once in a while.
class WXDLLIMPEXP_BASE wxExFile
{
public:
  /// Default constructor.
  /// The open_file parameter specifies the behaviour of FileLoad and
  /// FileSave, if true, the file is opened before calling DoFileLoad
  /// and DoFileSave, if false the file is not opened.
  /// That might be useful if you do not use the wxFile for loading
  /// and saving (as with XML documents), but still want to use the
  /// virtual file interface.
  wxExFile(bool open_file = true)
    : m_OpenFile(open_file)
    , m_File(std::make_unique<wxFile>()) {;};

  /// Constructor taking a filename.
  wxExFile(
    const wxExPath& filename,
    wxFile::OpenMode mode = wxFile::read,
    bool open_file = true)
    : m_File(std::make_unique<wxFile>(filename.Path().string(), mode))
    , m_Path(filename)
    , m_OpenFile(open_file)
    , m_Stat(filename.Path().string()) {
      MakeAbsolute();};
  
  /// Constructor taking a string filename.
  wxExFile(
    const std::string& filename,
    wxFile::OpenMode mode = wxFile::read,
    bool open_file = true)
    : wxExFile(wxExPath(filename), mode, open_file) {;};
  
  /// Copy constructor.
  wxExFile(const wxExFile& rhs) {*this = rhs; };
  
  /// Assignment operator.
  wxExFile& operator=(const wxExFile& f);

  /// Destructor, closes file if it was opened.
  virtual ~wxExFile() {};

  /// Checks whether this file can be synced, and 
  /// syncs (invokes DoFileLoad) the file if so.
  /// Returns true if this file was synced.
  bool CheckSync();

  /// Sets the filename member, opens the file if asked for,
  /// invokes DoFileLoad, and closes the file again.
  bool FileLoad(const wxExPath& filename);

  /// Sets the filename member and invokes DoFileNew.
  void FileNew(const wxExPath& filename);

  /// Sets the filename member if filename is ok, opens the file if asked for,
  /// invokes DoFileSave, and closes the file again.
  bool FileSave(const wxExPath& filename = wxExPath());

  /// Returns whether contents have been changed.
  virtual bool GetContentsChanged() const {return false;};

  /// Returns the file name.
  const auto & GetFileName() const {return m_Path;}
  
  /// Returns true if file is opened.
  bool IsOpened() const {return m_File->IsOpened();};

  /// Opens specified file.
  bool Open(const wxExPath& filename, 
    wxFile::OpenMode mode = wxFile::read, int access = wxS_DEFAULT);

  /// Opens current filename.
  bool Open(wxFile::OpenMode mode = wxFile::read, int access = wxS_DEFAULT)
    {return Open(m_Path, mode, access);};

  /// Reads this file into a buffer.
  const wxCharBuffer* Read(wxFileOffset seek_position = 0);

  /// Resets contents changed.
  virtual void ResetContentsChanged() {;};
  
  /// Writes file from buffer.
  bool Write(const wxCharBuffer& buffer) {
    return m_File->IsOpened() && 
      m_File->Write(buffer.data(), buffer.length()) == buffer.length();};
  
  /// Writes file from string.
  bool Write(const std::string& s) {
    return m_File->IsOpened() && m_File->Write(s);}; 
protected:
  /// Assigns the filename.
  /// Does not open the file, the filename does not need
  /// to exist. 
  /// Sets the is loaded member, so you can save the file
  /// from e.g. stc document (as with vcs blame).
  void Assign(const wxExPath& filename) {
    m_Path = filename;
    m_IsLoaded = true;
    m_Stat = filename.Path().string();};

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

  /// Returns length.
  wxFileOffset Length() const {return m_File->Length();};
private:
  bool Close() {return m_File->IsOpened() && m_File->Close();};
  bool Get(bool synced);
  void MakeAbsolute() {
    m_Path.MakeAbsolute();
    if (m_Path.m_Stat.Sync(m_Path.Path().string())) {
      m_Stat.Sync(m_Path.Path().string());};};
  
  bool m_IsLoaded = false;
  bool m_OpenFile;
  
  std::unique_ptr<wxCharBuffer> m_Buffer;
  std::unique_ptr<wxFile> m_File;

  wxExPath m_Path;
  wxExStat m_Stat; // used for syncing, no public access
};
