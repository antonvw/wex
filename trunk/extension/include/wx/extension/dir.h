/******************************************************************************\
* File:          dir.h
* Purpose:       Declaration of class 'exDir'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXDIR_H
#define _EXDIR_H

#include <wx/arrstr.h> 
#include <wx/dir.h>

/// Adds FindFiles to a wxDir, by overriding OnFile you can take care
/// of what to do with the result, and by overriding Cancelled to cancel traversing.
class exDir : public wxDir
{
public:
  /// Constructor.
  /// Opens the wxDir and sets the filespec.
  /// This filespec specifies what files are found.
  exDir(
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString); // finds all

  /// Destructor.
  virtual ~exDir();

  /// Allows you to cancel the FindFiles.
  virtual bool Cancelled() {return false;};

  /// Finds matching files.
  /// This results in recursive calls for OnFile if callOnFile is specified,
  /// this only makes sense if you implemented OnFile.
  size_t FindFiles(int flags = wxDIR_DEFAULT, bool callOnFile = true);

  /// Gets the files.
  const wxArrayString& GetFiles() const {return m_Files;};

  /// Gets the file spec.
  const wxString& GetFileSpec() const {return m_FileSpec;};

  /// Gets the flags.
  int GetFlags() const {return m_Flags;};
  
  /// Do something with the file.
  virtual void OnFile(const wxString& WXUNUSED(file)) {;};
private:
  const wxString m_FileSpec;
  int m_Flags;
  wxArrayString m_Files;
};
#endif
