/******************************************************************************\
* File:          support.h
* Purpose:       Declaration of support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: support.h 49 2008-11-12 19:08:42Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <wx/filetool/filetool.h>

/// Adds a toolbar and checkboxes to ftFrame.
class Frame : public ftFrame
{
public:
  /// Constructor.
  Frame(const wxString& project_wildcard);

#if wxUSE_CHECKBOX
  /// Gets hex mode.
  wxCheckBox* GetHexModeCheckBox() const {return m_HexModeCheckBox;};

  /// Gets sync mode.  
  wxCheckBox* GetSyncCheckBox() const {return m_SyncCheckBox;};
#endif

  /// Gets the toolbar.
  wxToolBar* GetToolBar() {return m_ToolBar;}
private:
  // Interface from exFrame.
  virtual bool AllowClose(wxWindowID id, wxWindow* page);
  virtual void OnNotebook(wxWindowID id, wxWindow* page);

#if wxUSE_CHECKBOX
  wxCheckBox* m_HexModeCheckBox;
  wxCheckBox* m_SyncCheckBox;
#endif
  exToolBar* m_ToolBar;
};
#endif
