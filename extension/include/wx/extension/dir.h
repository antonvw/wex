////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/extension/interruptable.h>
#include <wx/extension/stc-enums.h>

/// wxExDir flags, at this moment equal to wxDir flags,
/// will change after c++17.
enum wxExDirFlags
{
  DIR_FILES     = 0x0001, // include files
  DIR_DIRS      = 0x0002, // include directories
  DIR_HIDDEN    = 0x0004, // include hidden files

  DIR_DEFAULT   = DIR_FILES | DIR_DIRS | DIR_HIDDEN
};

class wxExDirImp;

/// Offers FindFiles method.
/// By overriding OnDir and OnFile you can take care
/// of what to do with the result.
class WXDLLIMPEXP_BASE wxExDir : public wxExInterruptable
{
public:
  /// Constructor.
  /// Opens the dir and sets the filespec.
  /// This filespec specifies what files are found.
  wxExDir(
    const std::string& dir,
    const std::string& filespec = std::string(),
    int flags = DIR_DEFAULT); // finds all

  /// Destructor.
  virtual ~wxExDir();

  /// Finds matching files.
  /// This results in recursive calls for OnDir and OnFile.
  /// Returns number of files matching, or -1 if error.
  int FindFiles();

  /// Returns the file spec.
  const auto & GetFileSpec() const {return m_FileSpec;};

  /// Returns the flags.
  int GetFlags() const {return m_Flags;};
 
  /// Returns true if the directory was successfully opened.
  bool IsOpened() const;

  /// Do something with the dir.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnDir(const std::string& ) {return true;};

  /// Do something with the file.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnFile(const std::string& ) {return true;};
private:
  std::unique_ptr<wxExDirImp> m_Dir;
  const std::string m_FileSpec;
  const int m_Flags;
};

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
    const std::string& fullpath,
    const std::string& filespec,
    wxExSTCWindowFlags file_flags = STC_WIN_DEFAULT,
    int dir_flags = DIR_DEFAULT);

  /// Opens each found file.
  virtual bool OnFile(const std::string& file) override;
private:
  wxExFrame* m_Frame;
  wxExSTCWindowFlags m_Flags;
};
#endif
