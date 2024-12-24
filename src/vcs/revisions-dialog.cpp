////////////////////////////////////////////////////////////////////////////////
// Name:      revisions-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/config.h>
#include <wex/factory/bind.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/listview.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs-entry.h>

namespace wex
{

void do_compare(
  vcs_entry*         ve,
  wex::listview*     lv,
  int                index,
  const std::string& repo_path,
  const path&        tl,
  const std::string& col)
{
  auto*       frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  const auto& value(lv->get_item_text(index, col));

  if (
    ve->system(process_data("diff -U0 " + value + " " + repo_path)
                 .start_dir(tl.string())) == 0)
  {
    unified_diff(path(repo_path), ve, frame).parse();
  }
};

void do_open(
  vcs_entry*         ve,
  wex::listview*     lv,
  int                index,
  const std::string& repo_path,
  const path&        tl,
  const std::string& col)
{
  auto*       frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  const auto& value(lv->get_item_text(index, col));

  config(ve->flags_key()).set(value);

  if (
    ve->system(
      process_data("show " + value + ":" + repo_path).start_dir(tl.string())) ==
    0)
  {
    frame->open_file_vcs(path(repo_path), *ve, data::stc());
    config(ve->flags_key()).set(std::string());
  }
};

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

void wex::vcs_entry::bind_rev(
  wex::listview*     lv,
  const std::string& repo_path,
  const path&        tl,
  const std::string& col)
{
  auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  lv->Bind(
    wxEVT_LEFT_DCLICK,
    [=, this](wxMouseEvent& event)
    {
      event.Skip();
      do_open(this, lv, lv->GetFirstSelected(), repo_path, tl, col);
    });

  bind(lv).command(
    {{[=, this](const wxCommandEvent& event)
      {
        for (auto i = lv->GetFirstSelected(); i != -1;
             i      = lv->GetNextSelected(i))
        {
          event.GetId() == ID_EDIT_REV_COMPARE ?
            do_compare(this, lv, i, repo_path, tl, col) :
            do_open(this, lv, i, repo_path, tl, col);
        }
      },
      ID_EDIT_REV_COMPARE,
      ID_EDIT_REV_OPEN}});
}

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
    const data::item di(
      data::item().is_persistent(false).label_type(data::item::LABEL_NONE));

    m_item_dialog = new item_dialog(
      {{"notebook",
        {{"versions",
          {{"vcs.hashes", data::listview().revision(true), std::any(), di}}},
         {"branches",
          {{"vcs.branches", data::listview().revision(true), std::any(), di}}},
         {"tags",
          {{"vcs.tags", data::listview().revision(true), std::any(), di}}}}}},
      data);

    is_new = true;
  }

  auto* vb =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.branches").window());
  auto* vt =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.tags").window());
  auto* lv =
    dynamic_cast<wex::listview*>(m_item_dialog->find("vcs.hashes").window());

  if (is_new)
  {
    vb->append_columns({{"branches", wex::column::STRING, 500}});
    vt->append_columns({{"tags", wex::column::STRING, 500}});
    lv->append_columns(
      {{"date", wex::column::STRING, 75},
       {"comment", wex::column::STRING, 400},
       {"author", wex::column::STRING},
       {"hash", wex::column::STRING}});

    bind_rev(vb, repo_path, tl, "branches");
    bind_rev(vt, repo_path, tl, "tags");
    bind_rev(lv, repo_path, tl, "hash");

    lv->field_separator('');
  }
  else
  {
    lv->clear();
    m_item_dialog->SetTitle(file.filename() + " " + _("Select Revision"));
  }

  vb->load(from_git(*this, "branch -a", 2));
  vt->load(from_git(*this, "tag")); // --sort=-creatordate

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
