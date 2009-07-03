/******************************************************************************\
* File:          dir.h
* Purpose:       Declaration of class 'wxExDir'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXDIR_H
#define _EXDIR_H

#include <wx/dir.h>

/// Adds FindFiles to a wxDir, by overriding OnFile you can take care
/// of what to do with the result, and by overriding Cancelled to cancel traversing.
class wxExDir : public wxDir
{
public:
  /// Constructor.
  /// Opens the wxDir and sets the filespec.
  /// This filespec specifies what files are found.
  wxExDir(
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString, // finds all
    int flags = wxDIR_DEFAULT);

  /// Destructor.
  virtual ~wxExDir() {;};

  /// Allows you to cancel the FindFiles.
  virtual bool Cancelled() {return false;};

  /// Finds matching files.
  /// This results in recursive calls for OnDir and OnFile.
  size_t FindFiles();

  /// Gets the file spec.
  const wxString& GetFileSpec() const {return m_FileSpec;};

  /// Gets the flags.
  int GetFlags() const {return m_Flags;};

  /// Do something with the dir.
  virtual void OnDir(const wxString& WXUNUSED(dir)){};

  /// Do something with the file.
  virtual void OnFile(const wxString& WXUNUSED(file)) = 0;
private:
  const wxString m_FileSpec;
  const int m_Flags;
  static bool m_IsBusy;
};
#endif
