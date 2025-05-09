////////////////////////////////////////////////////////////////////////////////
// Name:      grep.cpp
// Purpose:   Implementation of wex util method execute_grep
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/factory/process-data.h>
#include <wex/stc/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/item-build.h>
#include <wex/ui/item-dialog.h>
#include <wx/app.h>

namespace wex
{
bool execute_grep(const std::string& bin, const path& tl)
{
  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  auto* stc   = frame->get_stc();
  auto  text(stc->get_selected_text());
  static item_dialog* dlg = nullptr;

  if (text.empty())
  {
    if (dlg == nullptr)
    {
      dlg = new item_dialog(
        {add_find_text(),
         item::choices_bool_t{
           find_replace_data::get()->text_match_case(),
           find_replace_data::get()->text_match_word()}},
        wex::data::window().title("git grep"));
    }

    if (dlg->ShowModal() == wxID_CANCEL)
    {
      return false;
    }

    text = find_replace_data::get()->get_find_string();

    if (text.empty())
    {
      return false;
    }
  }
  else if (dlg != nullptr)
  {
    stc->get_find_string();
    dlg->reload();
  }

  if (const std::string &
        find(boost::algorithm::replace_all_copy(text, " ", "\\ "));
      find.contains("\n"))
  {
    log::status("Cannot grep multiple lines");
  }
  else
  {
    const std::string flag(
      config(find_replace_data::get()->text_match_case()).get(true) ? "" :
                                                                      " -i ");

    const std::string finds(
      config(find_replace_data::get()->text_match_word()).get(true) ?
        "\\b" + find + "\\b" :
        find);

    frame->process_async_system(
      process_data(bin + " grep -n " + flag + finds).start_dir(tl.string()));
  }

  return true;
}
} // namespace wex
