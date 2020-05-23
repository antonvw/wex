////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of class wex::report::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/accelerators.h>
#include <wex/bind.h>
#include <wex/config.h>
#include <wex/interruptable.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/listitem.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/frame.h>
#include <wex/report/listview.h>
#include <wex/report/stream.h>
#include <wex/util.h>
#include <wex/vcs.h>

wex::report::listview::listview(const listview_data& data)
  : wex::listview(data)
  , m_frame(dynamic_cast<report::frame*>(wxTheApp->GetTopWindow()))
  , m_menu_flags(data.menu())
{
  if (data.type() == listview_data::HISTORY)
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
    {{[=](wxCommandEvent& event) {
        bool           first = true;
        wxString       file1, file2;
        wex::listview* list = nullptr;
        for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
        {
          listitem         li(this, i);
          const wex::path* filename = &li.get_filename();
          if (!filename->file_exists())
            continue;
          switch (event.GetId())
          {
            case ID_LIST_COMPARE:
            {
              if (GetSelectedItemCount() == 1)
              {
                list = m_frame->activate(listview_data::FILE);
                if (list == nullptr)
                  return;
                const int main_selected = list->GetFirstSelected();
                compare_file(
                  listitem(list, main_selected).get_filename(),
                  *filename);
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
                  compare_file(path(file1), path(file2));
              }
            }
            break;
          }
        }
      },
      ID_LIST_COMPARE},
     {[=](wxCommandEvent& event) {
        make(listitem(this, GetFirstSelected()).get_filename());
      },
      ID_LIST_RUN_MAKE},
     {[=](wxCommandEvent& event) {
        const wex::tool& tool(event.GetId());
        if (
          tool.id() == ID_TOOL_REPORT_KEYWORD &&
          data.type() == listview_data::KEYWORD)
          return;
        if (
          tool.is_find_type() &&
          m_frame->find_in_files_dialog(tool.id()) == wxID_CANCEL)
          return;
        if (!report::stream::setup_tool(tool, m_frame))
          return;

#ifdef __WXMSW__
        std::thread t([=] {
#endif
          statistics<int> stats;

          for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
          {
            const listitem item(this, i);
            log::status() << item.get_filename();
            if (item.get_filename().file_exists())
            {
              stream file(item.get_filename(), tool);
              file.run_tool();
              stats += file.get_statistics().get_elements();
            }
            else
            {
              tool_dir dir(
                tool,
                item.get_filename().string(),
                item.file_spec());
              dir.find_files();
              stats += dir.get_statistics().get_elements();
            }
          }
          log::status(tool.info(&stats));
#ifdef __WXMSW__
        });
        t.detach();
#endif
      },
      ID_TOOL_LOWEST},
     {[=](wxCommandEvent& event) {
        std::vector<path> files;
        for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
        {
          files.emplace_back(listitem(this, i).get_filename().data());
        }
        vcs_execute(m_frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, files);
      },
      ID_EDIT_VCS_LOWEST}});
}

void wex::report::listview::build_popup_menu(wex::menu& menu)
{
  bool exists = true, is_folder = false, is_make = false, readonly = false;

  if (GetSelectedItemCount() >= 1)
  {
    const listitem item(this, GetFirstSelected());

    exists    = item.get_filename().stat().is_ok();
    is_folder = item.get_filename().dir_exists();
    readonly  = item.get_filename().stat().is_readonly();
    is_make   = item.get_filename().lexer().scintilla_lexer() == "makefile";
  }

  wex::listview::build_popup_menu(menu);

  if (
    GetSelectedItemCount() > 1 && exists &&
    !config(_("list.Comparator")).empty())
  {
    menu.append({{}, {ID_LIST_COMPARE, _("C&ompare") + "\tCtrl+O"}});
  }

  if (GetSelectedItemCount() == 1)
  {
    if (is_make)
    {
      menu.append({{}, {ID_LIST_RUN_MAKE, _("&Make")}});
    }

    if (
      data().type() != listview_data::FILE && !wex::vcs().use() && exists &&
      !is_folder)
    {
      if (auto* list = m_frame->activate(listview_data::FILE);
          list != nullptr && list->GetSelectedItemCount() == 1)
      {
        listitem       thislist(this, GetFirstSelected());
        const wxString current_file = thislist.get_filename().string();

        listitem otherlist(list, list->GetFirstSelected());

        if (const std::string with_file = otherlist.get_filename().string();
            current_file != with_file && !config(_("list.Comparator")).empty())
        {
          menu.append(
            {{},
             {ID_LIST_COMPARE,
              _("&Compare With") + " " + wxString(get_endoftext(with_file))}});
        }
      }
    }
  }

  if (GetSelectedItemCount() >= 1)
  {
    if (exists && !is_folder)
    {
      if (vcs::dir_exists(listitem(this, GetFirstSelected()).get_filename()))
      {
        bool restore = false;

        // The xml menus interprete is_selected for text parts,
        // so override.
        if (menu.style().test(menu::IS_SELECTED))
        {
          menu.style().set(menu::IS_SELECTED, false);
          restore = true;
        }

        menu.append({{}, {listitem(this, GetFirstSelected()).get_filename()}});

        if (restore)
        {
          menu.style().set(menu::IS_SELECTED);
        }
      }
    }

    // Finding in the listview_data::FIND would result in recursive calls, do
    // not add it.
    if (
      exists && data().type() != listview_data::FIND &&
      m_menu_flags.test(listview_data::MENU_REPORT_FIND))
    {
      menu.append(
        {{},
         {ID_TOOL_REPORT_FIND,
          ellipsed(m_frame->find_in_files_title(ID_TOOL_REPORT_FIND))}});

      if (!readonly)
      {
        menu.append(
          {{ID_TOOL_REPLACE,
            ellipsed(m_frame->find_in_files_title(ID_TOOL_REPLACE))}});
      }
    }
  }

  if (
    GetSelectedItemCount() > 0 && exists &&
    m_menu_flags.test(listview_data::MENU_TOOL) &&
    !lexers::get()->get_lexers().empty())
  {
    menu.append({{}, {menu_item::TOOLS}});
  }
}

bool wex::report::listview::Destroy()
{
  interruptable::cancel();
  return wex::listview::Destroy();
}

wex::listview_data::type_t wex::report::listview::type_tool(const tool& tool)
{
  switch (tool.id())
  {
    case ID_TOOL_REPLACE:
    case ID_TOOL_REPORT_FIND:
      return listview_data::FIND;

    case ID_TOOL_REPORT_KEYWORD:
      return listview_data::KEYWORD;

    default:
      return listview_data::NONE;
  }
}
