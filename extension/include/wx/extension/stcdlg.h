////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
    const wxString& text,
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
    
  /// Gets the STC lexer.
  const wxExLexer* GetLexer() const;
  
  /// Gets the STC.
  wxExSTC* GetSTC();

  /// Gets the normal STC text value.
  const wxString GetText() const;

  /// Gets raw STC text value.
  const wxCharBuffer GetTextRaw() const;

  /// Sets the STC lexer.
  bool SetLexer(const wxString& lexer);
private:
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
