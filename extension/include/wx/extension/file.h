////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <istream>
#include <memory>
#include <wx/extension/path.h>

namespace wex
{
  class file_imp;

  /// Offers several methods to read / write files. All the File* methods update
  /// the stat member. Also takes care of synchronization,
  /// all you have to do is call CheckSync once in a while.
  class file
  {
  public:
    /// Default constructor.
    /// The open_file parameter specifies the behaviour of FileLoad and
    /// FileSave, if true, the file is opened before calling DoFileLoad
    /// and DoFileSave, if false the file is not opened.
    file(bool open_file = true);

    /// Constructor taking a path, and opens the file.
    file(
      const path& p,
      std::ios_base::openmode mode = 
        std::ios_base::in | std::ios_base::out,
      bool open_file = true);
    
    /// Constructor taking a filename, and opens the file.
    file(
      const std::string& filename,
      std::ios_base::openmode mode =
        std::ios_base::in | std::ios_base::out,
      bool open_file = true);
    
    /// Copy constructor.
    file(const file& rhs);
    
    /// Destructor.
    virtual ~file();

    /// Assignment operator.
    file& operator=(const file& f);

    /// Checks whether this file can be synced, and 
    /// syncs (invokes DoFileLoad) the file if so.
    /// Returns true if this file was synced.
    bool CheckSync();

    /// Sets the path, opens the file if asked for,
    /// invokes DoFileLoad, and closes the file again.
    bool FileLoad(const path& p);

    /// Sets the path and invokes DoFileNew.
    bool FileNew(const path& p);

    /// Sets the path if path is ok, opens the file if asked for,
    /// invokes DoFileSave, and closes the file again.
    bool FileSave(const path& p = path());

    /// Returns the path.
    const path& GetFileName() const;
    
    /// Returns true if file is opened.
    bool IsOpened() const;

    /// Opens current path.
    bool Open(
      std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

    /// Opens specified path.
    bool Open(
      const path& p, 
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
    void Assign(const path& p);
    bool FileLoad(bool synced);
    
    bool m_IsLoaded {false}, m_OpenFile;
    
    std::unique_ptr<std::string> m_Buffer;
    std::unique_ptr<file_imp> m_File;
  };
};
