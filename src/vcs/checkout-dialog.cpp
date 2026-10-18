////////////////////////////////////////////////////////////////////////////////
// Name:      checkout-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/ui/item-dialog.h>
#include <wex/vcs/vcs-entry.h>

void wex::vcs_entry::checkout_dialog(const std::string& repo_path)
{
  const data::window data(
    data::window().size({350, 400}).title(_("Select Branch")));
  process_data datap;
  datap.start_dir(get_toplevel());

  m_checkout_dialog = new item_dialog(
    {{"branch",
      item::COMBOBOX,
      execute_and_parse(datap.exe(bin() + " branch -a"), 2)}},
    data);

  if (m_checkout_dialog->ShowModal() != wxID_CANCEL)
  {
    process pro;
    pro.system(
      datap.exe(bin() + " checkout " + config("branch").get_first_of()));
  }

  delete m_checkout_dialog;
}
