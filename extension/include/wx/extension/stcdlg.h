////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTCDLG_H
#define _EXSTCDLG_H

#include <wx/extension/dialog.h>
#include <wx/extension/shell.h>

#if wxUSE_GUI
class wxExLexer;

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
    /// normally a normal STC is used for entry,
    /// if you set this flag, an STCShell is used
    bool use_shell = false,
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

  /// Gets the STC Shell (only if use_shell was true when constructed).
  /// Therefore might be NULL.
  wxExSTCShell* GetSTCShell();

  /// Gets the normal STC text value.
  const wxString GetText() const;

  /// Gets raw STC text value.
  const wxCharBuffer GetTextRaw() const;

  /// Set the event handler for handling window close event.
  void SetEventHandler(wxEvtHandler* handler) {
    m_Handler = handler;};

  /// Sets the STC lexer.
  bool SetLexer(const wxString& lexer);
protected:
  void OnCommand(wxCommandEvent& command);
private:
  wxExSTC* m_STC;
  wxEvtHandler* m_Handler;

  DECLARE_EVENT_TABLE()
};

#endif // wxUSE_GUI
#endif
