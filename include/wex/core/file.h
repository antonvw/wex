////////////////////////////////////////////////////////////////////////////////
// Name:      file.h
// Purpose:   Declaration of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>

#include <fstream>
#include <istream>
#include <memory>

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
  file();

  /// Constructor taking a path.
  explicit file(const path& p);

  /// Constructor taking a filename.
  explicit file(const char* filename);

  /// Constructor taking a path, opens the file.
  file(const path& p, std::ios_base::openmode mode);

  /// Constructor taking a filename, opens the file.
  file(const char* filename, std::ios_base::openmode mode);

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

  /// Closes the file.
  bool close();

  /// Sets the path, opens the file if asked for,
  /// invokes do_file_load, and closes the file again (unless use_stream
  /// was invoked).
  bool file_load(const path& p);

  /// Sets the path and invokes do_file_new.
  bool file_new(const path& p);

  /// Sets the path if path is ok, opens the file if asked for,
  /// invokes do_file_save, and closes the file again.
  bool file_save(const wex::path& p = wex::path());

  /// Returns true if file is open.
  bool is_open() const { return m_fs.is_open(); }

  /// Returns true if file has been written.
  bool is_written() const { return m_is_written; }

  /// Opens current path.
  bool open(std::ios_base::openmode mode = std::ios_base::in)
  {
    return open(m_path, mode);
  };

  /// Opens specified path.
  bool open(const path& p, std::ios_base::openmode mode = std::ios_base::in);

  /// Returns the path.
  const auto& path() const { return m_path; }

  /// Writes char.
  void put(char c) { m_fs.put(c); }

  /// Reads this file into a buffer.
  const std::string* read(std::streampos seek_position = 0);

  /// Returns stream.
  std::fstream& stream() { return m_fs; }

  /// Default file is closed after loading, if you
  /// call this method, stream remains open.
  void use_stream(bool use = true) { m_use_stream = use; }

  /// Writes file from buffer.
  bool write(const char* s, size_t n);

  /// Writes file from string.
  bool write(const std::string& s) { return write(s.c_str(), s.size()); }

public:
  /// Returns whether contents have been changed.
  virtual bool is_contents_changed() const { return false; }

  /// Resets contents changed.
  virtual void reset_contents_changed() { ; }

protected:
  /// Invoked by file_load, allows you to load the file.
  /// The file is already opened, so you can call Read.
  /// If synced is true, this call was a result of
  /// check_sync and not of file_load.
  virtual bool do_file_load(bool synced = false) { return false; }

  /// Invoked by file_new, allows you to make a new (empty) file.
  virtual void do_file_new() { ; }

  /// Invoked by file_save, allows you to save the file.
  /// The file is already opened.
  /// The default save_as copies the file to new file.
  virtual void do_file_save(bool save_as = false);

private:
  void assign(const wex::path& p);
  bool file_load(bool synced);
  void log_stream_info(const std::string& info, size_t s);

  bool m_is_loaded{false}, m_is_written{false}, m_use_stream{false};

  wex::path                    m_path, m_path_prev;
  file_status                  m_stat; // used to check for sync
  std::fstream                 m_fs;
  std::unique_ptr<std::string> m_buffer;
};
} // namespace wex
