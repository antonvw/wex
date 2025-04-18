////////////////////////////////////////////////////////////////////////////////
// Name:      grep.cpp
// Purpose:   Implementation of wex util method execute_grep
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/log.h>
#include <wex/stc/entry-dialog.h>
#include <wex/stc/stc.h>
#include <wex/ui/frame.h>
#include <wx/app.h>

#include "util.h"

namespace wex
{
bool execute_grep(const std::string& bin, const path& tl)
{
  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  auto* stc   = frame->get_stc();
  auto  text(stc->get_selected_text());
  static stc_entry_dialog* dlg = nullptr;

  if (text.empty())
  {
    if (dlg == nullptr)
    {
      dlg = new stc_entry_dialog(
        std::string(),
        _("Text") + ":",
        wex::data::window().title("git grep"),
        wex::data::stc().flags(
          wex::data::stc::window_t().set(wex::data::stc::WIN_SINGLE_LINE)));
    }

    dlg->get_stc()->SetFocus();

    if (dlg->ShowModal() == wxID_CANCEL)
    {
      return false;
    }

    text = dlg->get_stc()->get_text();

    if (text.empty())
    {
      return false;
    }
  }
  else if (dlg != nullptr)
  {
    dlg->get_stc()->set_text(text);
  }

  if (const std::string &
        find(boost::algorithm::replace_all_copy(text, " ", "\\ "));
      find.contains("\n"))
  {
    log::status("Cannot grep multiple lines");
  }
  else
  {
    frame->process_async_system(
      process_data(bin + " grep -n " + find).start_dir(tl.string()));
  }

  return true;
}
} // namespace wex
