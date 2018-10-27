////////////////////////////////////////////////////////////////////////////////
// Name:      itemdlg.h
// Purpose:   Declaration of wex::item_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <wex/item.h>
#include <wex/itemtpldlg.h>

namespace wex
{
  /// Offers an item dialog.
  class item_dialog: public item_template_dialog <item>
  {
  public:
    /// Constructor.
    item_dialog(
      const std::vector< item >& v,
      const window_data& data = window_data().Title(_("Options").ToStdString()),
      int rows = 0,
      int cols = 1)
    : item_template_dialog(v, data, rows, cols) {
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_APPLY);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_CANCEL);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_CLOSE);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_OK);};

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
};
