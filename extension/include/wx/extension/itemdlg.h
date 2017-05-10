////////////////////////////////////////////////////////////////////////////////
// Name:      itemdlg.h
// Purpose:   Declaration of wxExItemDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <wx/extension/item.h>
#include <wx/extension/itemtpldlg.h>

#if wxUSE_GUI
/// Offers an item dialog.
class WXDLLIMPEXP_BASE wxExItemDialog: public wxExItemTemplateDialog <wxExItem>
{
public:
  /// Constructor.
  wxExItemDialog(
    const std::vector< wxExItem >& v,
    const wxExWindowData& data = wxExWindowData().Title(_("Options").ToStdString()),
    int rows = 0,
    int cols = 1)
  : wxExItemTemplateDialog(v, data, rows, cols) {
    Bind(wxEVT_BUTTON, &wxExItemDialog::OnCommand, this, wxID_APPLY);
    Bind(wxEVT_BUTTON, &wxExItemDialog::OnCommand, this, wxID_CANCEL);
    Bind(wxEVT_BUTTON, &wxExItemDialog::OnCommand, this, wxID_CLOSE);
    Bind(wxEVT_BUTTON, &wxExItemDialog::OnCommand, this, wxID_OK);};

  /// Reloads dialog from config.
  void Reload(bool save = false) const {
    for (const auto& it : GetItems())
    {
      it.ToConfig(save);
    }};
protected:
  void OnCommand(wxCommandEvent& event) {
    Reload(event.GetId() != wxID_CANCEL);
    event.Skip();};
};
#endif // wxUSE_GUI
