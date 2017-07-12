////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/interruptable.h>
#include <wx/extension/path.h>
#include <wx/extension/stc-enums.h>

/// wxExDir flags.
enum wxExDirFlags
{
  DIR_FILES     = 0x0001, // include files
  DIR_DIRS      = 0x0002, // include directories
  DIR_RECURSIVE = 0x0004, // recursive
  DIR_HIDDEN    = 0x0008, // include hidden files

  DIR_DEFAULT  = DIR_FILES | DIR_DIRS | DIR_RECURSIVE | DIR_HIDDEN
};

class wxExPath;

/// Offers FindFiles method.
/// By overriding OnDir and OnFile you can take care
/// of what to do with the result.
class WXDLLIMPEXP_BASE wxExDir : public wxExInterruptable
{
public:
  /// Constructor.
  wxExDir(
    /// the dir to start finding
    const wxExPath& path,
    /// the files to be found
    const std::string& filespec = std::string(),
    /// finds all
    int flags = DIR_DEFAULT); 

  /// Finds matching files.
  /// This results in recursive calls for OnDir and OnFile.
  /// Returns number of files matching, or -1 if error.
  int FindFiles();

  /// Returns the dir.
  const auto & GetDir() const {return m_Dir;};

  /// Returns the file spec.
  const auto & GetFileSpec() const {return m_FileSpec;};

  /// Returns the flags.
  int GetFlags() const {return m_Flags;};
 
  /// Do something with the dir.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnDir(const wxExPath& ) {return true;};

  /// Do something with the file.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnFile(const wxExPath& ) {return true;};
private:
  const wxExPath m_Dir;
  const std::string m_FileSpec;
  const int m_Flags;
};

/// Returns all matching files into a vector.
std::vector <wxExPath> wxExGetAllFiles(
  /// the dir to start finding
  const wxExPath& path,
  /// the files to be found
  const std::string& filespec = std::string(),
  /// finds all
  int flags = DIR_DEFAULT); 

#if wxUSE_GUI
class wxExFrame;

/// Allows you to easily open all files on specified path.
/// After constructing, invoke FindFiles which
/// causes all found files to be opened using OpenFile from frame.
class WXDLLIMPEXP_BASE wxExDirOpenFile : public wxExDir
{
public:
  /// Constructor.
  /// Flags are passed on to OpenFile, and dir flags for treating subdirs.
  wxExDirOpenFile(wxExFrame* frame,
    const wxExPath& path,
    const std::string& filespec,
    wxExSTCWindowFlags file_flags = STC_WIN_DEFAULT,
    int dir_flags = DIR_DEFAULT);

  /// Opens each found file.
  virtual bool OnFile(const wxExPath& file) override;
private:
  wxExFrame* m_Frame;
  wxExSTCWindowFlags m_Flags;
};
#endif
