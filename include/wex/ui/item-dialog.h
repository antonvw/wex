////////////////////////////////////////////////////////////////////////////////
// Name:      item-dialog.h
// Purpose:   Declaration of wex::item_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/ui/item-template-dialog.h>
#include <wex/ui/item.h>

import<utility>;

namespace wex
{
/// Offers an item dialog.
class item_dialog : public item_template_dialog<item>
{
public:
  /// Constructor.
  item_dialog(
    const std::vector<item>& v,
    const data::window& data = data::window().title(_("Options").ToStdString()),
    int                 rows = 0,
    int                 cols = 1)
    : item_template_dialog(v, data, rows, cols)
  {
    Bind(wxEVT_BUTTON, &item_dialog::on_command, this, wxID_APPLY);
    Bind(wxEVT_BUTTON, &item_dialog::on_command, this, wxID_CANCEL);
    Bind(wxEVT_BUTTON, &item_dialog::on_command, this, wxID_CLOSE);
    Bind(wxEVT_BUTTON, &item_dialog::on_command, this, wxID_OK);
  };

  /// Reloads dialog from config.
  void reload(bool save = false) const
  {
    for (const auto& it : get_items())
    {
      it.to_config(save);
    }
  };

private:
  void on_command(wxCommandEvent& event)
  {
    reload(event.GetId() != wxID_CANCEL);
    event.Skip();
  };
};
}; // namespace wex
