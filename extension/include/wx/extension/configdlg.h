////////////////////////////////////////////////////////////////////////////////
// Name:      configdlg.h
// Purpose:   Declaration of wxExConfigDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/configitem.h>
#include <wx/extension/itemtpldlg.h>

#if wxUSE_GUI
/// Offers an item dialog to set several items that load and
/// store to the config.
/// You can also use the dialog modeless (then you can use wxAPPLY
/// to store the items in the config).
class WXDLLIMPEXP_BASE wxExConfigDialog: public wxExItemTemplateDialog <wxExConfigItem>
{
public:
  /// Constructor.
  wxExConfigDialog(
    wxWindow* parent,
    const std::vector< wxExConfigItem >& v,
    const wxString& title = _("Options"),
    int rows = 0,
    int cols = -1,
    long flags = wxOK | wxCANCEL,
    wxWindowID id = wxID_ANY,
    int bookctrl_style = 0,
    wxImageList* imageList = nullptr,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize, 
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    const wxString& name = "wxExConfigItemDialog")
  : wxExItemTemplateDialog(parent, v, title, rows, cols, flags, id, bookctrl_style,
      imageList, pos, size, style, name) {
    Bind(wxEVT_BUTTON, &wxExConfigDialog::OnCommand, this, wxID_APPLY);
    Bind(wxEVT_BUTTON, &wxExConfigDialog::OnCommand, this, wxID_CANCEL);
    Bind(wxEVT_BUTTON, &wxExConfigDialog::OnCommand, this, wxID_CLOSE);
    Bind(wxEVT_BUTTON, &wxExConfigDialog::OnCommand, this, wxID_OK);};

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
