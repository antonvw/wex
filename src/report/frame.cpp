////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::report::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/bind.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/item-dialog.h>
#include <wex/lexers.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/report/defs.h>
#include <wex/report/dir.h>
#include <wex/report/frame.h>
#include <wex/report/listview-file.h>
#include <wex/report/stream.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/util.h>

wex::report::frame::frame(
  size_t              maxFiles,
  size_t              maxProjects,
  const data::window& data)
  : managed_frame(maxFiles, data)
  , m_project_history(maxProjects, ID_RECENT_PROJECT_LOWEST, "recent.Projects")
  , m_info(
      {find_replace_data::get()->text_match_word(),
       find_replace_data::get()->text_match_case(),
       find_replace_data::get()->text_regex()})
{
  std::set<std::string> t(m_info);
  t.insert(m_text_recursive + ",1");

  const std::vector<item> f{
    {find_replace_data::get()->text_find(),
     item::COMBOBOX,
     std::any(),
     data::control().is_required(true)},
    {m_text_in_files,
     item::COMBOBOX,
     default_extensions(),
     data::control().is_required(true)},
    {m_text_in_folder,
     item::COMBOBOX_DIR,
     std::list<std::string>{wxGetHomeDir().ToStdString()},
     data::control().is_required(true)},
    {t}};

  m_fif_dialog = new item_dialog(
    f,
    data::window()
      .button(wxAPPLY | wxCANCEL)
      .id(ID_FIND_IN_FILES)
      .title(_("Find In Files"))
      .style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  m_rif_dialog = new item_dialog(
    {f.at(0),
     {find_replace_data::get()->text_replace_with(), item::COMBOBOX},
     f.at(1),
     f.at(2),
     {_("fif.Max replacements"), -1, INT_MAX},
     // Match whole word does not work with replace.
     {{find_replace_data::get()->text_match_case(),
       find_replace_data::get()->text_regex(),
       m_text_recursive}}},
    data::window()
      .button(wxAPPLY | wxCANCEL)
      .id(ID_REPLACE_IN_FILES)
      .title(_("Replace In Files"))
      .style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  sync();

  Bind(wxEVT_CLOSE_WINDOW, [=, this](wxCloseEvent& event) {
    m_project_history.save();
    event.Skip();
  });

  bind(this).command(
    {{[=, this](wxCommandEvent& event) {
        m_project_history.clear();
      },
      ID_CLEAR_PROJECTS},
     {[=, this](wxCommandEvent& event) {
        if (auto* project = get_project(); project != nullptr)
        {
          project->file_save();
        }
      },
      ID_PROJECT_SAVE},
     {[=, this](wxCommandEvent& event) {
        if (!event.GetString().empty())
        {
          grep(event.GetString());
        }
        else if (m_fif_dialog != nullptr)
        {
          if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
          {
            m_fif_dialog->reload();
          }
          m_fif_dialog->Show();
        }
      },
      ID_TOOL_REPORT_FIND},
     {[=, this](wxCommandEvent& event) {
        if (!event.GetString().empty())
        {
          sed(event.GetString());
        }
        else if (m_rif_dialog != nullptr)
        {
          if (get_stc() != nullptr && !get_stc()->get_find_string().empty())
          {
            m_rif_dialog->reload();
          }
          m_rif_dialog->Show();
        }
      },
      ID_TOOL_REPLACE}});

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event) {
      on_menu_history(
        m_project_history,
        event.GetId() - m_project_history.get_base_id(),
        wex::data::stc::window_t().set(data::stc::WIN_IS_PROJECT));
    },
    m_project_history.get_base_id(),
    m_project_history.get_base_id() + m_project_history.get_max_files());
}

const std::string find_replace_string(bool replace)
{
  std::string log;

  log = _("Searching for") + ": " +
        wex::find_replace_data::get()->get_find_string();

  if (replace)
  {
    log += " " + _("replacing with") + ": " +
           wex::find_replace_data::get()->get_replace_string();
  }

  return log;
}

std::list<std::string> wex::report::frame::default_extensions() const
{
  std::list<std::string> l{std::string(wxFileSelectorDefaultWildcardStr)};

  for (const auto& it : lexers::get()->get_lexers())
  {
    if (!it.extensions().empty())
    {
      l.push_back(it.extensions());
    }
  }

  return l;
}

void wex::report::frame::find_in_files(wxWindowID dialogid)
{
  const bool      replace = (dialogid == ID_REPLACE_IN_FILES);
  const wex::tool tool    = (replace ? ID_TOOL_REPLACE : ID_TOOL_REPORT_FIND);

  if (!stream::setup_tool(tool, this))
    return;

#ifdef __WXMSW__
  std::thread t([=, this] {
#endif
    log::status(find_replace_string(replace));

    sync(false);

    data::dir::type_t type;
    type.set(data::dir::FILES);

    if (config(m_text_recursive).get(true))
    {
      type.set(data::dir::RECURSIVE);
    }

    find_replace_data::get()->set_regex(
      config(find_replace_data::get()->text_regex()).get(true));

    if (tool_dir dir(
          tool,
          config(m_text_in_folder).get_first_of(),
          data::dir()
            .file_spec(config(m_text_in_files).get_first_of())
            .type(type));

        dir.find_files() >= 0)
    {
      log::status(tool.info(&dir.get_statistics().get_elements()));
    }

    sync();

#ifdef __WXMSW__
  });
  t.detach();
#endif
}

bool wex::report::frame::find_in_files(
  const std::vector<path>& files,
  int                      id,
  bool                     show_dialog,
  wex::listview*           report)
{
  if (files.empty())
  {
    return false;
  }

  const wex::tool tool(id);

  if (const wex::path filename(files[0]);
      show_dialog &&
      find_in_files_dialog(
        tool.id(),
        filename.dir_exists() && !filename.file_exists()) == wxID_CANCEL)
  {
    return false;
  }

  if (!stream::setup_tool(tool, this, report))
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t([=, this] {
#endif
    statistics<int> stats;

    for (const auto& it : files)
    {
      if (it.file_exists())
      {
        if (stream file(it, tool); file.run_tool())
        {
          stats += file.get_statistics().get_elements();
        }
      }
      else if (it.dir_exists())
      {
        tool_dir dir(
          tool,
          it,
          data::dir().file_spec(config(m_text_in_files).get_first_of()));

        dir.find_files();
        stats += dir.get_statistics().get_elements();
      }
    }

    log::status(tool.info(&stats));

#ifdef __WXMSW__
  });
  t.detach();
#endif

  return true;
}

int wex::report::frame::find_in_files_dialog(int id, bool add_in_files)
{
  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (
    item_dialog(
      {{find_replace_data::get()->text_find(),
        item::COMBOBOX,
        std::any(),
        data::control().is_required(true)},
       (add_in_files ? item(
                         m_text_in_files,
                         item::COMBOBOX,
                         std::any(),
                         data::control().is_required(true)) :
                       item()),
       (id == ID_TOOL_REPLACE ?
          item(find_replace_data::get()->text_replace_with(), item::COMBOBOX) :
          item()),
       item(m_info)},
      data::window().title(find_in_files_title(id)))
      .ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  log::status(find_replace_string(id == ID_TOOL_REPLACE));

  return wxID_OK;
}

const std::string wex::report::frame::find_in_files_title(int id) const
{
  return (
    id == ID_TOOL_REPLACE ? _("Replace In Selection") : _("Find In Selection"));
}

bool wex::report::frame::grep(const std::string& arg, bool sed)
{
  static std::string       arg1 = config(m_text_in_folder).get_first_of();
  static std::string       arg2 = config(m_text_in_files).get_first_of();
  static data::dir::type_t arg3 = data::dir::type_t().set(data::dir::FILES);

  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (data::cmdline cmdl(arg);
      !cmdline(
         {{{"recursive,r", "recursive"},
           [&](bool on) {
             arg3.set(data::dir::RECURSIVE, on);
           }}},
         {},
         {{"rest",
           "match " + std::string(sed ? "replace" : "") +
             " [extension] [folder]"},
          [&](const std::vector<std::string>& v) {
            size_t i = 0;
            find_replace_data::get()->set_find_string(v[i++]);
            if (sed)
            {
              if (v.size() <= i)
                return;
              find_replace_data::get()->set_replace_string(v[i++]);
            }
            arg2 =
              (v.size() > i ? config(m_text_in_files).set_first_of(v[i++]) :
                              config(m_text_in_files).get_first_of());
            arg1 =
              (v.size() > i ? config(m_text_in_folder).set_first_of(v[i++]) :
                              config(m_text_in_folder).get_first_of());
          }},
         false,
         "grep")
         .parse(cmdl))
  {
    stc_entry_dialog(cmdl.help()).ShowModal();
    return false;
  }

  if (arg1.empty() || arg2.empty())
  {
    log::status("empty arguments") << arg1 << "or" << arg2;
    return false;
  }

  const wex::tool tool = (sed ? ID_TOOL_REPLACE : ID_TOOL_REPORT_FIND);

  if (!stream::setup_tool(tool, this))
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t([=, this] {
#endif
    if (auto* stc = get_stc(); stc != nullptr)
      path::current(stc->get_filename().get_path());
    find_replace_data::get()->set_regex(true);
    log::status(find_replace_string(false));
    sync(false);

    tool_dir dir(tool, arg1, data::dir().file_spec(arg2).type(arg3));
    dir.find_files();

    log::status(tool.info(&dir.get_statistics().get_elements()));
    sync(true);

#ifdef __WXMSW__
  });
  t.detach();
#endif

  return true;
}

void wex::report::frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptible::cancel())
      {
        log::status(_("Cancelled"));
      }
      break;

    case wxID_OK:
    case wxID_APPLY:
      switch (dialogid)
      {
        case wxID_ADD:
          if (get_project() != nullptr)
          {
            data::dir::type_t flags = 0;

            if (config(get_project()->text_addfiles()).get(true))
              flags.set(data::dir::FILES);
            if (config(get_project()->text_addrecursive()).get(true))
              flags.set(data::dir::RECURSIVE);
            if (config(get_project()->text_addfolders()).get(true))
              flags.set(data::dir::DIRS);

            get_project()->add_items(
              config(get_project()->text_infolder()).get_first_of(),
              config(get_project()->text_addwhat()).get_first_of(),
              flags);
          }
          break;

        case ID_FIND_IN_FILES:
        case ID_REPLACE_IN_FILES:
          find_in_files(dialogid);
          break;

        default:
          assert(0);
      }
      break;

    default:
      assert(0);
  }
}

void wex::report::frame::on_idle(wxIdleEvent& event)
{
  event.Skip();

  std::string       title(GetTitle());
  const std::string indicator(" *");

  if (title.size() < indicator.size())
  {
    return;
  }

  auto* stc     = get_stc();
  auto* project = get_project();

  if (const size_t pos = title.size() - indicator.size();
      (project != nullptr && project->is_contents_changed()) ||
      // using is_contents_changed gives assert in vcs dialog
      (stc != nullptr && stc->GetModify() &&
       !stc->data().flags().test(data::stc::WIN_NO_INDICATOR)))
  {
    // Project or editor changed, add indicator if not yet done.
    if (title.substr(pos) != indicator)
    {
      SetTitle(title + indicator);
    }
  }
  else
  {
    // Project or editor not changed, remove indicator if not yet done.
    if (title.substr(pos) == indicator)
    {
      SetTitle(title.erase(pos));
    }
  }
}

void wex::report::frame::set_recent_file(const wex::path& path)
{
  managed_frame::set_recent_file(path);

  if (m_file_history_listview != nullptr && path.file_exists())
  {
    listitem(m_file_history_listview, path).insert(0);

    if (m_file_history_listview->GetItemCount() > 1)
    {
      for (auto i = m_file_history_listview->GetItemCount() - 1; i >= 1; i--)
      {
        if (listitem item(m_file_history_listview, i);
            item.get_filename() == path)
        {
          item.erase();
        }
      }
    }
  }
}

void wex::report::frame::sync(bool start)
{
  start ? Bind(wxEVT_IDLE, &frame::on_idle, this) :
          (void)Unbind(wxEVT_IDLE, &frame::on_idle, this);
}
  
void wex::report::frame::use_file_history_list(wex::listview* list)
{
  assert(list->data().type() == data::listview::HISTORY);

  m_file_history_listview = list;
  m_file_history_listview->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < file_history().size(); i++)
  {
    if (listitem item(
          m_file_history_listview,
          file_history().get_history_file(i));
        item.get_filename().stat().is_ok())
    {
      item.insert();
    }
  }
}
