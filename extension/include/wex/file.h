////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <istream>
#include <memory>
#include <wex/path.h>

namespace wex
{
  class file_imp;

  /// Offers several methods to read / write files.
  /// Also takes care of synchronization,
  /// all you have to do is call check_sync once in a while.
  class file
  {
  public:
    /// Default constructor.
    /// The open_file parameter specifies the behaviour of file_load and
    /// file_save, if true, the file is opened before calling do_file_load
    /// and do_file_save, if false the file is not opened.
    file(bool open_file = true);

    /// Constructor taking a path, and opens the file.
    file(
      const path& p,
      std::ios_base::openmode mode = std::ios_base::in,
      bool open_file = true);
    
    /// Constructor taking a filename, and opens the file.
    file(
      const std::string& filename,
      std::ios_base::openmode mode = std::ios_base::in,
      bool open_file = true);
    
    /// Copy constructor.
    file(const file& rhs);
    
    /// Destructor.
    virtual ~file();

    /// Assignment operator.
    file& operator=(const file& f);

    /// Checks whether this file can be synced, and 
    /// syncs (invokes do_file_load) the file if so.
    /// Returns true if this file was synced.
    bool check_sync();

    /// Sets the path, opens the file if asked for,
    /// invokes do_file_load, and closes the file again.
    bool file_load(const path& p);

    /// Sets the path and invokes do_file_new.
    bool file_new(const path& p);

    /// Sets the path if path is ok, opens the file if asked for,
    /// invokes do_file_save, and closes the file again.
    bool file_save(const path& p = path());

    /// Returns the path.
    const path& get_filename() const;
    
    /// Returns true if file is opened.
    bool is_opened() const;

    /// Opens current path.
    bool open(
      std::ios_base::openmode mode = std::ios_base::in);

    /// Opens specified path.
    bool open(
      const path& p, 
      std::ios_base::openmode mode = std::ios_base::in);

    /// Reads this file into a buffer.
    const std::string* read(std::streampos seek_position = 0);

    /// Writes file from buffer.
    bool write(const char* s, size_t n);
    
    /// Writes file from string.
    bool write(const std::string& s);
  public:
    /// Returns whether contents have been changed.
    virtual bool get_contents_changed() const {return false;};

    /// Resets contents changed.
    virtual void reset_contents_changed() {;};
  protected:
    /// Invoked by file_load, allows you to load the file.
    /// The file is already opened, so you can call Read.
    /// If synced is true, this call was a result of
    /// check_sync and not of file_load.
    virtual bool do_file_load(bool synced = false) {return false;};

    /// Invoked by file_new, allows you to make a new (empty) file.
    virtual void do_file_new() {;};

    /// Invoked by file_save, allows you to save the file.
    /// The file is already opened.
    virtual void do_file_save(bool save_as = false) {;};
  private:
    void assign(const path& p);
    bool file_load(bool synced);
    
    bool m_IsLoaded {false}, m_open_file;
    
    std::unique_ptr<file_imp> m_File;
  };
};
