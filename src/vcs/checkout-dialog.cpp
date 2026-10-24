////////////////////////////////////////////////////////////////////////////////
// Name:      checkout-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>
#include <wex/vcs/vcs-entry.h>

#include "util.h"

namespace wex
{
void move_asterisk_to_front(strings_t& items)
{
  for (auto it = items.begin(); it != items.end(); ++it)
  {
    if (!it->empty() && it->front() == '*')
    {
      if (it != items.begin())
      {
        items.splice(items.begin(), items, it);
      }
      break;
    }
  }
}
} // namespace wex

void wex::vcs_entry::checkout_dialog(const std::string& repo_path)
{
  process_data data;
  data.start_dir(repo_path);
  const std::string key("branches");

  if (m_checkout_dialog == nullptr)
  {
    m_checkout_dialog = new item_dialog(
      {{key, item::COMBOBOX}},
      data::window().size({500, 400}).title(_("Checkout Branch")));
  }

  strings_t branches = execute_and_parse(data.exe(bin() + " branch -a"));

  move_asterisk_to_front(branches);

  combobox_from_list(
    dynamic_cast<wxComboBox*>(m_checkout_dialog->find(key).window()),
    branches);

  if (m_checkout_dialog->ShowModal() != wxID_CANCEL)
  {
    if (
      const auto& branch(
        rfind_after(config(key).get_first_of(), "/").substr(2));
      process().system(data.exe(bin() + " checkout " + branch)) == 0)
    {
      auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
      frame->statustext(branch, "PaneVCS");
    }
  }
}
