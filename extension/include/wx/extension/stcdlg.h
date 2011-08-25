////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTCDLG_H
#define _EXSTCDLG_H

#include <wx/extension/dialog.h> // for wxExDialog

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
    wxWindow* parent,
    const wxString& caption,
    const wxString& text,
    const wxString& prompt = wxEmptyString,
    long button_style = wxOK | wxCANCEL,
    bool use_shell = false,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
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

  /// Sets the STC text (either normal or raw).
  void SetText(const wxString& text);
protected:
  void OnCommand(wxCommandEvent& command);
private:
  wxExSTC* m_STC;

  DECLARE_EVENT_TABLE()
};

#endif // wxUSE_GUI
#endif
