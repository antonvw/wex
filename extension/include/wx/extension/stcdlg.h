////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/dialog.h>

#if wxUSE_GUI
class wxExLexer;
class wxExSTC;

/// Offers an wxExSTC as a dialog (like wxTextEntryDialog).
/// The prompt (if not empty) is first added as a text sizer to the user sizer.
/// Then the STC component is added to the user sizer.
class WXDLLIMPEXP_BASE wxExSTCEntryDialog : public wxExDialog
{
public:
  /// Constructor.
  wxExSTCEntryDialog(
    /// parent
    wxWindow* parent,
    /// caption
    const wxString& caption,
    /// initial text
    const std::string& text,
    /// prompt (as with wxTextEntryDialog)
    const wxString& prompt = wxEmptyString,
    /// buttons
    long button_style = wxOK | wxCANCEL,
    /// window id
    wxWindowID id = wxID_ANY,
    /// pos
    const wxPoint& pos = wxDefaultPosition,
    /// size
    const wxSize& size = wxDefaultSize, 
    /// dialog style
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    /// name
    const wxString& name = "wxExSTCEntryDialog");
    
  /// Returns the STC.
  wxExSTC* GetSTC() {return m_STC;};
private:
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
