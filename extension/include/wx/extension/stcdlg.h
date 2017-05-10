////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.h
// Purpose:   Declaration of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/dialog.h>
#include <wx/extension/window-data.h>

#if wxUSE_GUI
class wxExLexer;
class wxExSTC;

/// Offers an wxExSTC as a dialog (like wxTextEntryDialog).
/// The prompt (if not empty) is first added as a text sizer to the user sizer.
/// Then the STC component is added to the user sizer.
class WXDLLIMPEXP_BASE wxExSTCEntryDialog : public wxExDialog
{
public:
  /// Default constructor.
  wxExSTCEntryDialog(
    /// initial text
    const std::string& text = std::string(),
    /// prompt (as with wxTextEntryDialog)
    const std::string& prompt = std::string(),
    /// data
    const wxExWindowData& data = wxExWindowData());
    
  /// Returns the STC.
  wxExSTC* GetSTC() {return m_STC;};
private:
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
