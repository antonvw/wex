////////////////////////////////////////////////////////////////////////////////
// Name:      dir.h
// Purpose:   Declaration of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dir.h>
#include <wx/extension/interruptable.h>

/// Adds FindFiles to a wxDir.
/// By overriding OnDir and OnFile you can take care
/// of what to do with the result.
class WXDLLIMPEXP_BASE wxExDir : public wxDir, public wxExInterruptable
{
public:
  /// Constructor.
  /// Opens the wxDir and sets the filespec.
  /// This filespec specifies what files are found.
  wxExDir(
    const std::string& fullpath,
    const std::string& filespec = std::string(), // finds all
    int flags = wxDIR_DEFAULT);

  /// Destructor.
  virtual ~wxExDir() {;};

  /// Finds matching files.
  /// This results in recursive calls for OnDir and OnFile.
  /// Returns number of files matching, or -1 if error.
  int FindFiles();

  /// Returns the file spec.
  const auto & GetFileSpec() const {return m_FileSpec;};

  /// Returns the flags.
  int GetFlags() const {return m_Flags;};

  /// Do something with the dir.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnDir(const std::string& ){return true;};

  /// Do something with the file.
  /// Not made pure virtual, to allow this 
  /// class to be tested by calling FindFiles.
  virtual bool OnFile(const std::string& ){return true;};
private:
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
    long file_flags = 0,
    int dir_flags = wxDIR_DEFAULT);

  /// Opens each found file.
  virtual bool OnFile(const std::string& file) override;
private:
  wxExFrame* m_Frame;
  const long m_Flags;
};
#endif
