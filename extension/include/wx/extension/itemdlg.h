////////////////////////////////////////////////////////////////////////////////
// Name:      itemdlg.h
// Purpose:   Declaration of wxExItemDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
    wxWindow* parent,
    const std::vector< wxExItem >& v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = 1,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = "wxExItemDialog")
  : wxExItemTemplateDialog(parent, v, title, rows, cols, flags, id,
      pos, size, style, name) {
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
