////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Created:   2009-11-18
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTCDLG_H
#define _EXSTCDLG_H

#include <wx/extension/dialog.h> // for wxExDialog

#if wxUSE_GUI
class wxExSTC;

/// Offers an wxExSTC as a dialog (like wxTextEntryDialog).
/// The prompt is allowed to be empty, in that case no sizer is used for it.
class wxExSTCEntryDialog : public wxExDialog
{
public:
  /// Constructor.
  wxExSTCEntryDialog(
    wxWindow* parent,
    const wxString& caption,
    const wxString& text,
    const wxString& prompt = wxEmptyString,
    long button_style = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = wxDialogNameStr);

  /// Gets the STC scintilla lexer.
  const wxString GetLexer() const;

  /// Gets the normal text value.
  const wxString GetText() const;

  /// Gets raw text value.
  wxCharBuffer GetTextRaw() const;

  /// Sets the STC lexer.
  void SetLexer(const wxString& lexer);

  /// Sets the text (either normal or raw).
  /// Resets a previous lexer if asked for.
  void SetText(const wxString& text, bool reset_lexer = true);
private:
  wxExSTC* m_STC;
};

#endif // wxUSE_GUI
#endif
