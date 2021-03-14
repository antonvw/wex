////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/accelerators.h>
#include <wex/addressrange.h>
#include <wex/bind.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/ctags.h>
#include <wex/debug.h>
#include <wex/file-dialog.h>
#include <wex/frd.h>
#include <wex/item-dialog.h>
#include <wex/lexers.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/regex.h>
#include <wex/del/defs.h>
#include <wex/del/dir.h>
#include <wex/del/frame.h>
#include <wex/del/listview-file.h>
#include <wex/del/stream.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/textctrl.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs.h>

namespace wex
{
  bool is_ex(textctrl* tc)
  {
    return tc != nullptr && tc->stc() != nullptr && !tc->stc()->is_visual();
  }
} // namespace wex

wex::del::frame::frame(
  size_t              maxFiles,
  size_t              maxProjects,
  const data::window& data)
  : managed_frame(maxFiles, data)
  , m_debug(new debug(this))
  , m_project_history(maxProjects, ID_RECENT_PROJECT_LOWEST, "recent.Projects")
  , m_info(
      {find_replace_data::get()->text_match_word(),
       find_replace_data::get()->text_match_case(),
       find_replace_data::get()->text_regex()})
{
  std::set<std::string> t(m_info);
  t.insert(m_text_recursive + ",1");

  accelerators({{wxACCEL_NORMAL, WXK_F5, wxID_FIND},
                {wxACCEL_NORMAL, WXK_F6, wxID_REPLACE},
                {wxACCEL_CTRL, 'I', ID_VIEW_MENUBAR},
                {wxACCEL_CTRL, 'T', ID_VIEW_TITLEBAR}})
    .set(this);

  vcs::load_document();
  ex::get_macros().load_document();

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
        if (GetStatusBar() != nullptr)
        {
          GetStatusBar()->Show(!GetStatusBar()->IsShown());
          SendSizeEvent();
        }
      },
      ID_VIEW_STATUSBAR},
     {[=, this](wxCommandEvent& event) {
        m_is_command = true;

        // :e [+command] [file]
        if (!event.GetString().empty())
        {
          std::string text(event.GetString());

          if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
          {
            wex::path::current(stc->get_filename().get_path());
            if (!marker_and_register_expansion(&stc->get_ex(), text))
              return;
          }

          if (!shell_expansion(text))
            return;

          std::string cmd;
          if (regex v("\\+([^ \t]+)* *(.*)"); v.match(text) > 1)
          {
            cmd  = v[0];
            text = v[1];
          }

          open_files(
            this,
            to_vector_path(text).get(),
            data::control().command(cmd));
        }
        else
        {
          data::window data;
          data.style(
            wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR | wxFD_HEX_MODE);
          open_files_dialog(this, false, data::stc(data));
        }
      },
      wxID_OPEN},
     {[=, this](wxCommandEvent& event) {
        SetMenuBar(GetMenuBar() != nullptr ? nullptr : m_menubar);
      },
      ID_VIEW_MENUBAR},
     {[=, this](wxCommandEvent& event) {
        SetWindowStyleFlag(
          !(GetWindowStyleFlag() & wxCAPTION) ?
            wxDEFAULT_FRAME_STYLE :
            GetWindowStyleFlag() & ~wxCAPTION);
        Refresh();
      },
      ID_VIEW_TITLEBAR}});

  bind(this).command(
    {{[=, this](wxCommandEvent& event) {
        find_replace_data::get()->set_find_strings(std::list<std::string>{});
      },
      ID_CLEAR_FINDS},
     {[=, this](wxCommandEvent& event) {
        stc::config_dialog(data::window()
                             .id(wxID_PREFERENCES)
                             .parent(this)
                             .title(_("Editor Options"))
                             .button(wxAPPLY | wxOK | wxCANCEL));
      },
      wxID_PREFERENCES},
     {[=, this](wxCommandEvent& event) {
        file_history().clear();
      },
      ID_CLEAR_FILES},
     {[=, this](wxCommandEvent& event) {
        if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
        {
          auto it = find_replace_data::get()->get_find_strings().begin();
          std::advance(it, event.GetId() - ID_FIND_FIRST);
          if (const std::string text(*it); stc->find(
                text,
                stc->get_ex().is_active() ? stc->get_ex().search_flags() : -1))
          {
            find_replace_data::get()->set_find_string(text);
          }
        }
      },
      ID_FIND_FIRST}});

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

wex::del::frame::~frame()
{
  stc::on_exit();
  addressrange::on_exit();
  ctags::close();

  delete m_debug;
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

std::list<std::string> wex::del::frame::default_extensions() const
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

void wex::del::frame::find_in_files(wxWindowID dialogid)
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

bool wex::del::frame::find_in_files(
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

int wex::del::frame::find_in_files_dialog(int id, bool add_in_files)
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

const std::string wex::del::frame::find_in_files_title(int id) const
{
  return (
    id == ID_TOOL_REPLACE ? _("Replace In Selection") : _("Find In Selection"));
}

bool wex::del::frame::grep(const std::string& arg, bool sed)
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
    if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
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

void wex::del::frame::on_command_item_dialog(
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
          if (auto* p = get_project(); p != nullptr)
          {
            data::dir::type_t flags = 0;

            if (config(p->text_addfiles()).get(true))
              flags.set(data::dir::FILES);
            if (config(p->text_addrecursive()).get(true))
              flags.set(data::dir::RECURSIVE);
            if (config(p->text_addfolders()).get(true))
              flags.set(data::dir::DIRS);

            p->add_items(
              config(p->text_infolder()).get_first_of(),
              config(p->text_addwhat()).get_first_of(),
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

void wex::del::frame::on_idle(wxIdleEvent& event)
{
  event.Skip();

  std::string       title(GetTitle());
  const std::string indicator(" *");

  if (title.size() < indicator.size())
  {
    return;
  }

  auto* stc     = dynamic_cast<wex::stc*>(get_stc());
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

void wex::del::frame::on_notebook(wxWindowID id, wxWindow* page)
{
  if (auto* stc = dynamic_cast<wex::stc*>(page); stc != nullptr)
  {
    show_ex_bar(
      !stc->is_visual() ? SHOW_BAR : HIDE_BAR_FOCUS_STC,
      &stc->get_ex());

    set_recent_file(stc->get_filename());

    const vcs v({stc->get_filename()});

    if (const auto& b(v.get_branch()); !b.empty())
    {
      statustext(b, "PaneVCS");
    }
    else
    {
      statustext(v.name(), "PaneVCS");
    }
  }
}

void wex::del::frame::set_recent_file(const wex::path& path)
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

wex::factory::stc* wex::del::frame::open_file(
  const wex::path& filename,
  const vcs_entry& vcs,
  const data::stc& data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->set_text(vcs.get_stdout());
    vcs_command_stc(vcs.get_command(), filename.lexer(), stc);
    return stc;
  }

  return nullptr;
}

void wex::del::frame::print_ex(ex* ex, const std::string& text)
{
  ex->print(text);
}

void wex::del::frame::show_ex_bar(int action, ex* ex)
{
  if (action == SHOW_BAR || ex != nullptr)
  {
    if (action >= SHOW_BAR)
    {
      m_textctrl->set_stc(ex->get_stc(), ":");
    }

    pane_show("VIBAR", action >= SHOW_BAR);
  }
  else
  {
    if (
      action == HIDE_BAR_FORCE || action == HIDE_BAR_FORCE_FOCUS_STC ||
      (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      pane_show("VIBAR", false);
    }

    if (
      (action == HIDE_BAR_FOCUS_STC || action == HIDE_BAR_FORCE_FOCUS_STC) &&
      m_textctrl->stc() != nullptr)
    {
      m_textctrl->stc()->SetFocus();
    }
  }
}

void wex::del::frame::show_ex_message(const std::string& text)
{
  if (!is_ex(m_textctrl))
  {
    show_ex_bar();
  }

  statustext(text, std::string());
}

void wex::del::frame::statusbar_clicked(const std::string& pane)
{
  if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); pane == "PaneDBG")
  {
    if (get_debug()->show_dialog(this))
    {
      statustext(get_debug()->debug_entry().name(), pane);
    }
  }
  else if (pane == "PaneVCS")
  {
    if (wex::vcs::size() > 0)
    {
      auto* menu = new wex::menu;

      if (stc != nullptr)
      {
        if (menu->append({{stc->get_filename().get_path()}}))
        {
          PopupMenu(menu);
        }
      }
      else if (menu->append({{wex::path()}}))
      {
        PopupMenu(menu);
      }

      delete menu;
    }
  }
  else if (pane == "PaneLexer")
  {
    if (stc != nullptr && lexers_dialog(stc))
    {
      statustext(stc->get_lexer().display_lexer(), "PaneLexer");
    }
  }
  else if (pane == "PaneFileType")
  {
    if (stc != nullptr)
      stc->filetype_menu();
  }
  else if (pane == "PaneMacro")
  {
    if (stc != nullptr)
      stc->get_vi().get_macros().mode().transition("@", &stc->get_vi(), true);
  }
  else
  {
    managed_frame::statusbar_clicked(pane);
  }
}

void wex::del::frame::statusbar_clicked_right(const std::string& pane)
{
  if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); pane == "PaneInfo")
  {
    if (stc != nullptr)
    {
      PopupMenu(new wex::menu(
        {{wxWindow::NewControlId(),
          stc->is_shown_line_numbers() ? "&Hide" : "&Show",
          data::menu().action([=, this](wxCommandEvent&) {
            stc->show_line_numbers(!stc->is_shown_line_numbers());
          })}}));
    }
  }
  else if (pane == "PaneMacro")
  {
    if (wex::ex::get_macros().get_filename().file_exists())
    {
      managed_frame::open_file(
        wex::ex::get_macros().get_filename(),
        wex::data::control().find(
          !get_statustext(pane).empty() ?
            " name=\"" + get_statustext(pane) + "\"" :
            std::string()));
    }
  }
  else if (pane == "PaneDBG" || pane == "PaneVCS")
  {
    std::string match(get_statustext(pane));

    if (stc != nullptr)
    {
      match =
        (pane == "PaneVCS" ?
           wex::vcs({stc->get_filename().string()}).entry().name() :
           wex::debug(this).debug_entry().name());
    }

    managed_frame::open_file(
      wex::menus::get_filename(),
      wex::data::control().find(match));
  }
  else
  {
    managed_frame::statusbar_clicked_right(pane);
  }
}

void wex::del::frame::sync(bool start)
{
  start ? Bind(wxEVT_IDLE, &frame::on_idle, this) :
          (void)Unbind(wxEVT_IDLE, &frame::on_idle, this);
}

void wex::del::frame::use_file_history_list(wex::listview* list)
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
