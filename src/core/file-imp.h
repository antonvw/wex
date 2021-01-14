////////////////////////////////////////////////////////////////////////////////
// Name:      file_imp.h
// Purpose:   Declaration of class wex::file_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <fstream>
#include <wex/path.h>
#include <wex/stat.h>

namespace wex
{
  class file_imp
  {
  public:
    /// Default constructor.
    file_imp() { ; };

    /// Constructor from path.
    file_imp(const path& filename);

    /// Constructor from path and mode.
    file_imp(const path& filename, std::ios_base::openmode mode);

    /// Destructor.
    virtual ~file_imp() { ; };

    /// Assigns path.
    void assign(const path& p);

    /// Closes file.
    bool close();

    /// Returns true if this file has been written.
    bool is_written() const { return m_is_written; };

    /// Opens file.
    bool open(const path& p, std::ios_base::openmode mode = std::ios_base::in);

    /// Returns path.
    auto& path() { return m_path; };

    // Puts char.
    void put(char c) { m_fs.put(c); };

    /// Reads from file into string.
    const std::string* read(std::streampos seek_position);

    /// Returns stat.
    auto& stat() { return m_stat; };

    /// Returns stream.
    auto& stream() { return m_fs; };

    /// Writes file.
    bool write(const char* s, size_t n);

  private:
    void log_stream_info(const std::string& info, size_t s);

    wex::path                    m_path;
    file_stat                    m_stat; // used to check for sync
    bool                         m_is_written{false};
    std::fstream                 m_fs;
    std::unique_ptr<std::string> m_buffer;
  };
}; // namespace wex
