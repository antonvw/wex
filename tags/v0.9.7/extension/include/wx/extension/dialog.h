/******************************************************************************\
* File:          dialog.h
* Purpose:       Declaration of wxExDialog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXDIALOG_H
#define _EXDIALOG_H

#include <wx/dialog.h>
#include <wx/sizer.h>

#if wxUSE_GUI

/// Offers a general dialog, with a separated button sizer at the bottom.
/// Derived dialogs can use the user sizer for laying out their controls.
class WXDLLIMPEXP_BASE wxExDialog : public wxDialog
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
    const wxString& name = "wxExDialog");
protected:
  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxWindow* window,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxSizer* sizer,
    const wxSizerFlags& flags = wxSizerFlags().Expand().Center());

  /// Gets the button flags (as specified in the constructor).
  long GetButtonFlags() const {return m_ButtonFlags;};

  /// Layouts the sizers. Should be invoked after adding to sizers.
  /// If you specified button flags,
  /// they will be put at the bottom of the top sizer,
  /// and a sepator line will be added as specified.
  void LayoutSizers(bool add_separator_line = true);
private:
  const long m_ButtonFlags;
  wxFlexGridSizer* m_TopSizer;
  wxFlexGridSizer* m_UserSizer;
};
#endif // wxUSE_GUI
#endif
