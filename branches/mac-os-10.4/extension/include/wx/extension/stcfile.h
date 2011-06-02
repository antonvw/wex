/******************************************************************************\
* File:          stcfile.h
* Purpose:       Declaration of class wxExSTCFile
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSTCFILE_H
#define _EXSTCFILE_H

#include <wx/extension/file.h> // for wxExFile

class wxExSTC;

#if wxUSE_GUI
/// Adds file read and write to wxExSTC.
class WXDLLIMPEXP_BASE wxExSTCFile: public wxExFile
{
public:
  /// Constructor.
  /// Does not open the file.
  wxExSTCFile(wxExSTC* stc);
  
  /// Override virtual methods.
  virtual bool GetContentsChanged() const;
  virtual void ResetContentsChanged();
  
  /// Reads other file and adds contents to STC.
  void Read(const wxString& file) const;
protected:
  virtual void DoFileLoad(bool synced = false);
  virtual void DoFileNew();
  virtual void DoFileSave(bool save_as = false);
private:
  void AddBasePathToPathList();
  void ReadFromFile(bool get_only_new_data);

  wxExSTC* m_STC;
  wxFileOffset m_PreviousLength;
};
#endif // wxUSE_GUI
#endif
