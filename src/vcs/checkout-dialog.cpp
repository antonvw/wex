////////////////////////////////////////////////////////////////////////////////
// Name:      checkout-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/syntax/util.h>
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

  auto branches = execute_and_parse(data.exe(bin() + " branch -a"));
  move_asterisk_to_front(branches);

  if (
    std::string selection; single_choice_dialog(
      data::window().title(_("Checkout Branch")),
      branches,
      selection))
  {
    if (
      const auto& branch(rfind_after(selection.substr(2), "/"));
      process().system(data.exe(bin() + " checkout " + branch)) == 0)
    {
      auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
      frame->statustext(branch, "PaneVCS");
    }
  }
}
