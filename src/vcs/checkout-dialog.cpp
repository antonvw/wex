////////////////////////////////////////////////////////////////////////////////
// Name:      checkout-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/ui/item-dialog.h>
#include <wex/vcs/vcs-entry.h>

#include "util.h"

void wex::vcs_entry::checkout_dialog(const std::string& repo_path)
{
  process_data data;
  data.start_dir(repo_path);

  m_checkout_dialog = new item_dialog(
    {{"branches",
      item::COMBOBOX,
      execute_and_parse(data.exe(bin() + " branch -a"), 2)}},
    data::window().size({350, 400}).title(_("Select Branch")));

  if (m_checkout_dialog->ShowModal() != wxID_CANCEL)
  {
    process pro;
    pro.system(
      data.exe(bin() + " checkout " + config("branches").get_first_of()));
  }

  delete m_checkout_dialog;
}
