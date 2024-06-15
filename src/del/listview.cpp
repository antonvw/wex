////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of class wex::del::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>
#include <wx/app.h>

namespace wex::del
{
struct menu_env
{
  bool is_ok = true, is_folder = false, is_build = false, is_readonly = false;
};
} // namespace wex::del

wex::del::listview::listview(const data::listview& data)
  : wex::listview(data)
  , m_frame(dynamic_cast<del::frame*>(wxTheApp->GetTopWindow()))
  , m_menu_flags(data.menu())
{
  if (data.type() == data::listview::HISTORY)
  {
    m_frame->use_file_history_list(this);
  }

  accelerators({{wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE},
                {wxACCEL_CTRL, WXK_INSERT, wxID_COPY},
                {wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE},
                {wxACCEL_SHIFT, WXK_DELETE, wxID_CUT},
                {wxACCEL_CTRL, 'O', ID_LIST_COMPARE}})
    .set(this);

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        on_compare();
      },
      ID_LIST_COMPARE},

     {[=, this](wxCommandEvent& event)
      {
        build(path_lexer(listitem(this, GetFirstSelected()).path()));
      },
      ID_LIST_RUN_BUILD},

     {[=, this](wxCommandEvent& event)
      {
        on_tool(event);
      },
      ID_TOOL_LOWEST},

     {[=, this](wxCommandEvent& event)
      {
        std::vector<path> files;
        for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
        {
          files.emplace_back(listitem(this, i).path().data());
        }
        vcs_execute(m_frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, files);
      },
      ID_EDIT_VCS_LOWEST}});
}

void wex::del::listview::build_popup_menu(wex::menu& menu)
{
  menu_env env;

  if (const auto index = GetFirstSelected(); index != -1)
  {
    const listitem item(this, index);

    env.is_ok       = item.path().stat().is_ok();
    env.is_folder   = item.path().dir_exists();
    env.is_readonly = item.path().stat().is_readonly();
    env.is_build    = path_lexer(item.path()).is_build();
  }

  wex::listview::build_popup_menu(menu);

  if (
    GetSelectedItemCount() > 1 && env.is_ok &&
    !config(_("list.Comparator")).empty())
  {
    menu.append({{}, {ID_LIST_COMPARE, _("C&ompare") + "\tCtrl+O"}});
  }

  if (GetSelectedItemCount() == 1)
  {
    build_popup_menu_single(&env, menu);
  }

  if (GetSelectedItemCount() >= 1)
  {
    build_popup_menu_multiple(&env, menu);
  }
}

void wex::del::listview::build_popup_menu_multiple(
  const menu_env* env,
  wex::menu&      menu)
{
  if (env->is_ok && !env->is_folder)
  {
    if (vcs::dir_exists(listitem(this, GetFirstSelected()).path()))
    {
      bool restore = false;

      // The xml menus is_selected for text parts,
      // so override.
      if (menu.style().test(menu::IS_SELECTED))
      {
        menu.style().set(menu::IS_SELECTED, false);
        restore = true;
      }

      menu.append({{}, {listitem(this, GetFirstSelected()).path(), m_frame}});

      if (restore)
      {
        menu.style().set(menu::IS_SELECTED);
      }
    }
  }

  // Finding in the data::listview::FIND would result in recursive calls, do
  // not add it.
  if (
    env->is_ok && data().type() != data::listview::FIND &&
    m_menu_flags.test(data::listview::MENU_REPORT_FIND))
  {
    menu.append(
      {{},
       {ID_TOOL_REPORT_FIND,
        ellipsed(m_frame->find_in_files_title(ID_TOOL_REPORT_FIND))}});

    if (!env->is_readonly)
    {
      menu.append(
        {{ID_TOOL_REPLACE,
          ellipsed(m_frame->find_in_files_title(ID_TOOL_REPLACE))}});
    }
  }
}

void wex::del::listview::build_popup_menu_single(
  const menu_env* env,
  wex::menu&      menu)
{
  if (env->is_build)
  {
    menu.append({{}, {ID_LIST_RUN_BUILD, _("&Build")}});
  }

  if (data().type() != data::listview::FILE && env->is_ok && !env->is_folder)
  {
    if (auto* list = m_frame->activate(data::listview::FILE);
        list != nullptr && list->GetSelectedItemCount() == 1)
    {
      listitem    thislist(this, GetFirstSelected());
      const auto& current_file = thislist.path().string();

      listitem otherlist(list, list->GetFirstSelected());

      if (const auto& with_file = otherlist.path().string();
          current_file != with_file && !config(_("list.Comparator")).empty())
      {
        menu.append(
          {{},
           {ID_LIST_COMPARE,
            _("&Compare With").ToStdString() + " " + find_tail(with_file)}});
      }
    }
  }
}

bool wex::del::listview::Destroy()
{
  interruptible::end();
  return wex::listview::Destroy();
}

void wex::del::listview::on_compare()
{
  bool           first = true;
  std::string    file1, file2;
  wex::listview* list = nullptr;

  for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
  {
    listitem         li(this, i);
    const wex::path* filename = &li.path();

    if (!filename->file_exists())
    {
      continue;
    }

    if (GetSelectedItemCount() == 1)
    {
      list = m_frame->activate(data::listview::FILE);

      if (list == nullptr)
      {
        return;
      }

      const int main_selected = list->GetFirstSelected();
      compare_file(listitem(list, main_selected).path(), *filename);
    }
    else
    {
      if (first)
      {
        first = false;
        file1 = filename->string();
      }
      else
      {
        first = true;
        file2 = filename->string();
      }
      if (first)
      {
        compare_file(path(file1), path(file2));
      }
    }
  }
}

void wex::del::listview::on_tool(const wxCommandEvent& event)
{
  const wex::tool tool((wex::window_id)event.GetId());

  if (tool.is_find_type() && m_frame->find_in_files_dialog(tool) == wxID_CANCEL)
  {
    return;
  }

  if (auto* lv = m_frame->activate(listview::type_tool(tool)); lv != nullptr)
  {
    statistics<int> stats;

    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      const listitem item(this, i);
      log::status() << item.path();

      if (item.path().file_exists())
      {
        wex::stream file(find_replace_data::get(), item.path(), tool, lv);
        file.run_tool();
        stats += file.get_statistics().get_elements();
      }
      else
      {
        wex::dir dir(item.path(), data::dir().file_spec(item.file_spec()), lv);
        dir.find_files(tool);
        stats += dir.get_statistics().get_elements();
      }
    }

    log::status(tool.info(&stats));
  }
}

wex::data::listview::type_t wex::del::listview::type_tool(const tool& tool)
{
  switch (tool.id())
  {
    case ID_TOOL_REPLACE:
    case ID_TOOL_REPORT_FIND:
      return data::listview::FIND;

    default:
      return data::listview::NONE;
  }
}
