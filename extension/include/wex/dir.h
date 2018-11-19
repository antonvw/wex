////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wex::dir and wex::open_file_dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wex/interruptable.h>
#include <wex/path.h>
#include <wex/stc-data.h>

namespace wex
{
  class path;

  /// Offers find_files method.
  /// By overriding on_dir and on_file you can take care
  /// of what to do with the result.
  class dir : public interruptable
  {
  public:
    /// dir flags.
    enum
    {
      FILES     = 0, // include files
      DIRS      = 1, // include directories
      RECURSIVE = 2, // recursive
      HIDDEN    = 3, // include hidden files
    };

    typedef std::bitset<4> type_t;
    
    /// Constructor.
    dir(
      /// the dir to start finding
      const path& path,
      /// the files to be found
      const std::string& filespec = std::string(),
      /// finds all
      type_t flags = type_t().set()); 

    /// Returns the file spec.
    const auto & file_spec() const {return m_FileSpec;};

    /// Finds matching files.
    /// This results in recursive calls for on_dir and on_file.
    /// Returns number of files matching, or -1 if error.
    int find_files();

    /// Returns the path.
    const auto & get_path() const {return m_Dir;};

    /// Returns the flags.
    auto type() const {return m_Flags;};
   
    /// Do something with the dir.
    /// Not made pure virtual, to allow this 
    /// class to be tested by calling find_files.
    virtual bool on_dir(const path& ) {return true;};

    /// Do something with the file.
    /// Not made pure virtual, to allow this 
    /// class to be tested by calling find_files.
    virtual bool on_file(const path& ) {return true;};
  private:
    const path m_Dir;
    const std::string m_FileSpec;
    const type_t m_Flags;
  };

  /// Returns all matching files into a vector of paths.
  std::vector <path> get_all_files(
    /// the dir to start finding
    const path& path,
    /// the files to be found
    const std::string& filespec = std::string(),
    /// finds all
    dir::type_t flags = dir::type_t().set()); 

  /// Returns all matching files into a vector of strings (without paths).
  std::vector <std::string> get_all_files(
    /// the dir to start finding
    const std::string& path,
    /// the files to be found
    const std::string& filespec = std::string(),
    /// finds all
    dir::type_t flags = dir::type_t().set());

  class frame;

  /// Allows you to easily open all files on specified path.
  /// After constructing, invoke find_files which
  /// causes all found files to be opened using open_file from frame.
  class open_file_dir : public dir
  {
  public:
    /// Constructor.
    /// Flags are passed on to open_file, and dir flags for treating subdirs.
    open_file_dir(frame* frame,
      const path& path,
      const std::string& filespec,
      stc_data::window_t file_flags = 0,
      type_t type = dir::type_t().set());

    /// Opens each found file.
    virtual bool on_file(const path& file) override;
  private:
    frame* m_Frame;
    stc_data::window_t m_Flags;
  };
};
