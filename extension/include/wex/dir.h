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

  /// Offers FindFiles method.
  /// By overriding OnDir and OnFile you can take care
  /// of what to do with the result.
  class dir : public interruptable
  {
  public:
    /// dir flags.
    enum dir_flags
    {
      FILES     = 0x0001, // include files
      DIRS      = 0x0002, // include directories
      RECURSIVE = 0x0004, // recursive
      HIDDEN    = 0x0008, // include hidden files

      DEFAULT = FILES | DIRS | RECURSIVE | HIDDEN
    };

    /// Constructor.
    dir(
      /// the dir to start finding
      const path& path,
      /// the files to be found
      const std::string& filespec = std::string(),
      /// finds all
      int flags = DEFAULT); 

    /// Finds matching files.
    /// This results in recursive calls for OnDir and OnFile.
    /// Returns number of files matching, or -1 if error.
    int FindFiles();

    /// Returns the dir.
    const auto & GetDir() const {return m_Dir;};

    /// Returns the file spec.
    const auto & GetFileSpec() const {return m_FileSpec;};

    /// Returns the flags.
    auto GetFlags() const {return m_Flags;};
   
    /// Do something with the dir.
    /// Not made pure virtual, to allow this 
    /// class to be tested by calling FindFiles.
    virtual bool OnDir(const path& ) {return true;};

    /// Do something with the file.
    /// Not made pure virtual, to allow this 
    /// class to be tested by calling FindFiles.
    virtual bool OnFile(const path& ) {return true;};
  private:
    const path m_Dir;
    const std::string m_FileSpec;
    const int m_Flags;
  };

  /// Returns all matching files into a vector of paths.
  std::vector <path> get_all_files(
    /// the dir to start finding
    const path& path,
    /// the files to be found
    const std::string& filespec = std::string(),
    /// finds all
    int flags = dir::DEFAULT); 

  /// Returns all matching files into a vector of strings (without paths).
  std::vector <std::string> get_all_files(
    /// the dir to start finding
    const std::string& path,
    /// the files to be found
    const std::string& filespec = std::string(),
    /// finds all
    int flags = dir::FILES | dir::DIRS); 

  class frame;

  /// Allows you to easily open all files on specified path.
  /// After constructing, invoke FindFiles which
  /// causes all found files to be opened using OpenFile from frame.
  class open_file_dir : public dir
  {
  public:
    /// Constructor.
    /// Flags are passed on to OpenFile, and dir flags for treating subdirs.
    open_file_dir(frame* frame,
      const path& path,
      const std::string& filespec,
      stc_data::window_flags file_flags = stc_data::WIN_DEFAULT,
      int dir_flags = dir::DEFAULT);

    /// Opens each found file.
    virtual bool OnFile(const path& file) override;
  private:
    frame* m_Frame;
    stc_data::window_flags m_Flags;
  };
};
