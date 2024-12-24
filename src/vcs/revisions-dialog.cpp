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
class rev_data
{
public:
  rev_data(
    vcs_entry*         ve,
    wex::listview*     lv,
    long               index,
    const path&        tl,
    const std::string& repo_path,
    const std::string& col)
    : m_ve(ve)
    , m_lv(lv)
    , m_index(index)
    , m_tl(tl)
    , m_repo_path(repo_path)
    , m_col(col)
  {
    ;
  }

  void do_compare();
  void do_open();

private:
  auto* frame() const
  {
    return dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
  };

  const auto value() const { return m_lv->get_item_text(m_index, m_col); };

  vcs_entry*         m_ve;
  wex::listview*     m_lv;
  long               m_index;
  const path&        m_tl;
  const std::string &m_repo_path, m_col;
};

void rev_data::do_compare()
{
  if (
    m_ve->system(process_data("diff -U0 " + value() + " " + m_repo_path)
                   .start_dir(m_tl.string())) == 0)
  {
    unified_diff(path(m_repo_path), m_ve, frame()).parse();
  }
};

void rev_data::do_open()
{
  config(m_ve->flags_key()).set(value());

  if (
    m_ve->system(process_data("show " + value() + ":" + m_repo_path)
                   .start_dir(m_tl.string())) == 0)
  {
    frame()->open_file_vcs(path(m_repo_path), *m_ve, data::stc());
    config(m_ve->flags_key()).set(std::string());
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
  lv->Bind(
    wxEVT_LEFT_DCLICK,
    [=, this](wxMouseEvent& event)
    {
      event.Skip();
      rev_data(this, lv, lv->GetFirstSelected(), tl, repo_path, col).do_open();
    });

  bind(lv).command(
    {{[=, this](const wxCommandEvent& event)
      {
        for (auto i = lv->GetFirstSelected(); i != -1;
             i      = lv->GetNextSelected(i))
        {
          event.GetId() == ID_EDIT_REV_COMPARE ?
            rev_data(this, lv, i, tl, repo_path, col).do_compare() :
            rev_data(this, lv, i, tl, repo_path, col).do_open();
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
