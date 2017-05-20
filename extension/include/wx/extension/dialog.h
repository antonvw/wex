////////////////////////////////////////////////////////////////////////////////
// Name:      dialog.h
// Purpose:   Declaration of wxExDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/extension/window-data.h>

#if wxUSE_GUI

/// Offers a general dialog, with a separated button sizer at the bottom.
/// Derived dialogs can use the user sizer for laying out their controls.
class WXDLLIMPEXP_BASE wxExDialog : public wxDialog
{
public:
  /// Default constructor.
  wxExDialog(const wxExWindowData& data = wxExWindowData());

  /// Returns the window data.
  const auto& GetData() const {return m_Data;};
protected:
  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxWindow* window,
    const wxSizerFlags& flags = wxSizerFlags().Expand());

  /// Adds to the user sizer using the sizer flags.
  wxSizerItem* AddUserSizer(
    wxSizer* sizer,
    const wxSizerFlags& flags = wxSizerFlags().Expand());

  /// Layouts the sizers. Should be invoked after adding to sizers.
  /// If you specified button flags,
  /// they will be put at the bottom of the top sizer,
  /// and a sepator line will be added as specified.
  void LayoutSizers(bool add_separator_line = true);
private:
  const wxExWindowData m_Data;
  
  wxFlexGridSizer* m_TopSizer;
  wxFlexGridSizer* m_UserSizer;
};
#endif // wxUSE_GUI
