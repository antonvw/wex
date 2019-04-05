////////////////////////////////////////////////////////////////////////////////
// Name:      itemdlg.h
// Purpose:   Declaration of wex::item_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
      const window_data& data = window_data().title(_("Options").ToStdString()),
      int rows = 0,
      int cols = 1)
    : item_template_dialog(v, data, rows, cols) {
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_APPLY);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_CANCEL);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_CLOSE);
      Bind(wxEVT_BUTTON, &item_dialog::OnCommand, this, wxID_OK);};

    /// reloads dialog from config.
    void reload(bool save = false) const {
      for (const auto& it : get_items())
      {
        it.to_config(save);
      }};
  private:
    void OnCommand(wxCommandEvent& event) {
      reload(event.GetId() != wxID_CANCEL);
      event.Skip();};
  };
};
