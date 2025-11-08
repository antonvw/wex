////////////////////////////////////////////////////////////////////////////////
// Name:      revisions-dialog.cpp
// Purpose:   Implementation of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
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
    const process_data& data,
    const std::string& repo_path,
    const std::string& col)
    : m_ve(ve)
    , m_lv(lv)
    , m_index(index)
    , m_data(data)
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

  const auto value() const { return m_lv->get_item_text(m_index, m_col); }

  vcs_entry*           m_ve;
  const wex::listview* m_lv;
  const long           m_index;
  const process_data&  m_data;
  const std::string &  m_repo_path, m_col;
};

void rev_data::do_compare()
{
  if (
    m_ve->system(process_data(m_data).exe("diff -U0 " + value() + " " + m_repo_path)) == 0)
  {
    unified_diff(path(m_repo_path), m_ve, frame()).parse();
  }
};

void rev_data::do_open()
{
  config(m_ve->flags_key()).set(value());

  if (
    m_ve->system(process_data(m_data).exe("show " + value() + ":" + m_repo_path)) == 0)
  {
    frame()->open_file_vcs(path(m_repo_path), *m_ve, data::stc());
    config(m_ve->flags_key()).set(std::string());
  }
};

} // namespace wex

void wex::vcs_entry::bind_rev(
  wex::listview*      lv,
  const std::string&  repo_path,
  const process_data& data,
  const std::string&  col)
{
  lv->Bind(
    wxEVT_LEFT_DCLICK,
    [=, this](wxMouseEvent& event)
    {
      event.Skip();
      rev_data(this, lv, lv->GetFirstSelected(), data, repo_path, col).do_open();
    });

  bind(lv).command(
    {{[=, this](const wxCommandEvent& event)
      {
        for (auto i = lv->GetFirstSelected(); i != -1;
             i      = lv->GetNextSelected(i))
        {
          event.GetId() == ID_EDIT_REV_COMPARE ?
            rev_data(this, lv, i, data, repo_path, col).do_compare() :
            rev_data(this, lv, i, data, repo_path, col).do_open();
        }
      },
      ID_EDIT_REV_COMPARE,
      ID_EDIT_REV_OPEN}});
}

wex::strings_t wex::vcs_entry::execute_and_parse(const process_data& data, size_t offset)
{
  process pro;
  pro.system(data);

  strings_t values{};

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
    vb->append_columns({{"branches", wex::column::STRING_LARGE}});
    vt->append_columns({{"tags", wex::column::STRING_MEDIUM}});
    lv->append_columns(
      {{"date", wex::column::DATE},
       {"comment", wex::column::STRING_LARGE},
       {"author", wex::column::STRING_MEDIUM},
       {"hash", wex::column::STRING_SMALL}});

    lv->field_separator('');
  }
  else
  {
    lv->clear();
    m_item_dialog->SetTitle(file.filename() + " " + _("Select Revision"));
  }

  process_data data;
  data.start_dir(tl.string());

  bind_rev(vb, repo_path, data, "branches");
  bind_rev(vt, repo_path, data, "tags");
  bind_rev(lv, repo_path, data, "hash");

  vb->load(execute_and_parse(data.exe(bin() + " branch -a"), 2));
  vt->load(execute_and_parse(data.exe(bin() + " tag"))); // --sort=-creatordate

  process pro;
  pro.system(
    // this query should follow the columns as specified above,
    // and using same field separator as used for the listview
    data.exe(bin() + " log --date=short --pretty=format:%ad%s%an%h " + repo_path));
  lv->item_from_text(pro.std_out());

  return m_item_dialog->Show();
}
