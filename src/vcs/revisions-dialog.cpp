////////////////////////////////////////////////////////////////////////////////
// Name:      revisions-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/config.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/listview.h>
#include <wex/vcs/vcs-entry.h>

#define BIND_LISTVIEW(LV, COL)                                                 \
  LV->Bind(                                                                    \
    wxEVT_LEFT_DCLICK,                                                         \
    [=, this](wxMouseEvent& event)                                             \
    {                                                                          \
      event.Skip();                                                            \
      const auto  index(LV->GetFirstSelected());                               \
      const auto& value(LV->get_item_text(index, COL));                        \
      config(flags_key()).set(value);                                          \
      if (                                                                     \
        system(process_data("show " + value + ":" + repo_path)                 \
                 .start_dir(tl.string())) == 0)                                \
      {                                                                        \
        frame->open_file_vcs(path(repo_path), *this, data::stc());             \
        config(flags_key()).set(std::string());                                \
      }                                                                        \
    })

namespace wex
{
strings_t
from_git(const vcs_entry& e, const std::string& ask, size_t offset = 0)
{
  process pro;
  pro.system(process_data(e.bin() + " " + ask));

  strings_t values;

  values.emplace_back(std::string());

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         pro.std_out(),
         boost::char_separator<char>("\r\n")))
  {
    if (const auto& line(it); !line.contains("HEAD ->"))
    {
      values.emplace_back(offset == 0 ? line : line.substr(offset));
    }
  }

  return values;
}
} // namespace wex

int wex::vcs_entry::revisions_dialog(
  const std::string& repo_path,
  const path&        tl,
  const path&        file)
{
  bool is_new{false};

  if (m_item_dialog == nullptr)
  {
    const data::window data(
      data::window()
        .title(file.filename() + " " + _("Select Revision"))
        .size({350, 400}));

    m_item_dialog = new item_dialog(
      {{"notebook",
        {{"versions", {{"vcs.hashes", data::listview()}}},
         {"branches", {{"vcs.branches", data::listview()}}},
         {"tags", {{"vcs.tags", data::listview()}}}}}},
      data);

    is_new = true;
  }

  auto* vb =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.branches").window());
  auto* vt =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.tags").window());
  auto* lv =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.hashes").window());
  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());

  if (is_new)
  {
    vb->append_columns({{"branches", wex::column::STRING, 500}});
    vt->append_columns({{"tags", wex::column::STRING, 500}});
    lv->append_columns(
      {{"date", wex::column::STRING, 75},
       {"comment", wex::column::STRING, 400},
       {"author", wex::column::STRING},
       {"hash", wex::column::STRING}});

    BIND_LISTVIEW(vb, "branches");
    BIND_LISTVIEW(vt, "tags");
    BIND_LISTVIEW(lv, "hash");

    lv->field_separator('');
  }
  else
  {
    lv->clear();
    m_item_dialog->SetTitle(file.filename() + " " + _("Select Revision"));
  }

  vb->load(from_git(*this, "tag")); // --sort=-creatordate
  vt->load(from_git(*this, "branch -a", 2));

  process pro;
  pro.system(
    // this query should follow the columns as specified above,
    // and using same field separator as used for the listview
    process_data(
      bin() + " log --date=short --pretty=format:%ad%s%an%h " + repo_path)
      .start_dir(tl.string()));
  lv->item_from_text(pro.std_out());

  return m_item_dialog->Show();
}
