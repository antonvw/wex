/******************************************************************************\
* File:          dialog.h
* Purpose:       Declaration of wxWidgets base extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: base.h 1647 2009-09-17 09:52:27Z antonvw $
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXDIALOG_H
#define _EXDIALOG_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dialog.h>

// Only if we have a gui.
#if wxUSE_GUI

/// Offers a general dialog, with a separated button sizer at the bottom.
/// Derived dialogs can use the user sizer for laying out their controls.
class wxExDialog : public wxDialog
{
public:
  /// Constructor.
  /// Flags is a bit list of the following flags:
  /// wxOK, wxCANCEL, wxYES, wxNO, wxAPPLY, wxCLOSE, wxHELP, wxNO_DEFAULT.
  wxExDialog(wxWindow* parent,
    const wxString& title,
    long button_flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);
protected:
  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxWindow* window,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxSizer* sizer,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// BUild the sizers. Should be invoked after adding to sizers.
  void BuildSizers();

  /// Gets the flags (as specified in constructor).
  long GetButtonFlags() const {return m_ButtonFlags;};
private:
  const long m_ButtonFlags;
  wxFlexGridSizer* m_TopSizer;
  wxFlexGridSizer* m_UserSizer;
};
#endif // wxUSE_GUI
#endif
