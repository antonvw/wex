////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <wex/wex.h>
#include <wx/timer.h>

#include "blaming.h"

#define POPUPMENU_DO(ON, ACTION)                                               \
  if (stc != nullptr)                                                          \
  {                                                                            \
    PopupMenu(new wex::menu(                                                   \
      {{wxWindow::NewControlId(),                                              \
        stc->ON() ? "&Hide" : "&Show",                                         \
        data::menu().action(                                                   \
          [=, this](wxCommandEvent&)                                           \
          {                                                                    \
            stc->ACTION(!stc->ON());                                           \
          })}}));                                                              \
  }

namespace wex::del
{
/// The lowest number of projects to be supported.
const int ID_RECENT_PROJECT_LOWEST = wxID_FILE1 + NUMBER_RECENT_FILES + 1;

const std::string find_replace_string(bool replace)
{
  return std::string(_("Searching for") + ": ") +
         wex::find_replace_data::get()->get_find_string() +
         (replace ? std::string(
                      " " + _("replacing with") + ": " +
                      wex::find_replace_data::get()->get_replace_string()) :
                    std::string());
}

const std::string get_some_text(const std::vector<std::string>& text)
{
  std::stringstream ss;
  const std::string info(!text.front().empty() ? text.front() : std::string());
  ss << "deleted " << text.size() << " lines" << info;
  return ss.str();
}

bool is_ex(ex_commandline* cl)
{
  return cl->stc() != nullptr && !cl->stc()->is_visual();
}
} // namespace wex::del

wex::del::frame::frame(
  size_t              maxFiles,
  size_t              maxProjects,
  const data::window& data)
  : wex::frame(maxFiles, data)
  , m_debug(new debug(this))
  , m_project_history(maxProjects, ID_RECENT_PROJECT_LOWEST, "recent.Projects")
  , m_info(
      {find_replace_data::get()->text_match_case(),
       find_replace_data::get()->text_regex(),
       m_text_recursive + ",1",
       m_text_hidden})
  , m_vcs(new wex::vcs())
  , m_function_repeat(
      "frame",
      this,
      [this](wxTimerEvent&)
      {
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
      })
{
  auto info(m_info);
  // Match whole word does not work with replace.
  info.insert(find_replace_data::get()->text_match_word());

  accelerators({{wxACCEL_NORMAL, WXK_F5, wxID_FIND},
                {wxACCEL_NORMAL, WXK_F6, wxID_REPLACE},
                {wxACCEL_CTRL, 'I', ID_VIEW_MENUBAR},
                {wxACCEL_CTRL, 'T', ID_VIEW_TITLEBAR}})
    .set(this);

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
     config::strings_t{wxGetHomeDir()},
     data::control().is_required(true)},
    {info}};

  m_fif_dialog = new item_dialog(
    f,
    data::window()
      .button(wxAPPLY | wxCANCEL)
      .id(id_find_in_files)
      .title(_("Find In Files"))
#ifdef __WXOSX__
      .size({375, 350})
#endif
      .style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  m_rif_dialog = new item_dialog(
    {f.at(0),
     add_combobox_with_max(
       find_replace_data::get()->text_replace_with(),
       _("fif.Max")),
     f.at(1),
     f.at(2),
     m_info},
    data::window()
      .button(wxAPPLY | wxCANCEL)
      .id(id_replace_in_files)
      .title(_("Replace In Files"))
#ifdef __WXOSX__
      .size({410, 400})
#endif
      .style(wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxSTAY_ON_TOP));

  sync(true);

  bind_all();
}

wex::del::frame::~frame() {}

wex::del::listview* wex::del::frame::activate_and_clear(const wex::tool& tool)
{
  auto* lv = activate(listview::type_tool(tool));

  if (lv != nullptr)
  {
    lv->clear();
  }

  return lv;
}

void append_submenu(const wex::menu_item* item, wex::menu* menu)
{
  wex::menu* submenu(menu);

  if (menu->style().test(wex::menu::IS_POPUP))
  {
    submenu = new wex::menu(menu->style());
  }

  if (const wex::vcs vcs({item->path()});
      vcs.entry().build_menu(wex::ID_EDIT_VCS_LOWEST + 1, submenu))
  {
    if (menu->style().test(wex::menu::IS_POPUP))
    {
      menu->append({{submenu, vcs.entry().name()}});
    }
  }
}

void wex::del::frame::bind_accelerators(
  wxWindow*                              parent,
  const std::vector<wxAcceleratorEntry>& v,
  bool                                   debug)
{
  accelerators(v, debug).set(parent);
}

void wex::del::frame::debug_add_menu(menu& m, bool b)
{
  m_debug->add_menu(&m, b);
}

void wex::del::frame::debug_exe(int id, syntax::stc* stc)
{
  m_debug->execute(id, dynamic_cast<wex::stc*>(stc));
}

void wex::del::frame::debug_exe(const std::string& exe, syntax::stc* stc)
{
  m_debug->execute(exe, dynamic_cast<wex::stc*>(stc));
}

wxEvtHandler* wex::del::frame::debug_handler()
{
  return m_debug;
}

bool wex::del::frame::debug_is_active() const
{
  return m_debug->is_active();
}

bool wex::del::frame::debug_print(const std::string& text)
{
  return m_debug->print(text);
}

bool wex::del::frame::debug_toggle_breakpoint(int line, syntax::stc* stc)
{
  return m_debug->toggle_breakpoint(line, dynamic_cast<wex::stc*>(stc));
}

wex::config::strings_t wex::del::frame::default_extensions() const
{
  config::strings_t l{std::string(wxFileSelectorDefaultWildcardStr)};

  for (const auto& it : lexers::get()->get_lexers())
  {
    if (!it.extensions().empty())
    {
      l.emplace_back(it.extensions());
    }
  }

  return l;
}

wex::stc_entry_dialog*
wex::del::frame::entry_dialog(const std::string& title, const std::string& text)
{
  if (m_entry_dialog == nullptr)
  {
    m_entry_dialog = new stc_entry_dialog(
      text,
      std::string(),
      data::window().title(title),
      data::stc(data::window().size({350, 250})));
  }
  else
  {
    if (!text.empty())
    {
      m_entry_dialog->get_stc()->set_text(text);
    }

    if (!title.empty())
    {
      m_entry_dialog->SetTitle(title);
    }
  }

  return m_entry_dialog;
}

bool wex::del::frame::grep(const std::string& arg, bool sed)
{
  static auto arg1 = config(m_text_in_folder).get_first_of();
  static auto arg2 = config(m_text_in_files).get_first_of();
  static auto arg3 = data::dir::type_t().set(data::dir::FILES);

  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (data::cmdline cmdl(arg);
      !cmdline(
         {{{"hidden,H", "hidden"},
           [&](bool on)
           {
             arg3.set(data::dir::HIDDEN, on);
           }},
          {{"recursive,r", "recursive"},
           [&](bool on)
           {
             arg3.set(data::dir::RECURSIVE, on);
           }}},
         {},
         {{"rest",
           "match " + std::string(sed ? "replace" : "") +
             " [extension] [folder]"},
          [&](const std::vector<std::string>& v)
          {
            size_t i = 0;
            find_replace_data::get()->set_find_string(v[i++]);
            if (sed)
            {
              if (v.size() <= i)
              {
                return;
              }
              find_replace_data::get()->set_replace_string(v[i++]);
            }
            arg2 =
              (v.size() > i ? config(m_text_in_files).set_first_of(v[i++]) :
                              config(m_text_in_files).get_first_of());
            arg1 =
              (v.size() > i ? config(m_text_in_folder).set_first_of(v[i++]) :
                              config(m_text_in_folder).get_first_of());
          }},
         false)
         .parse(cmdl))
  {
    statustext(cmdl.help(), std::string());
    statustext(std::string(), std::string());
    entry_dialog(!sed ? "grep" : "sed", cmdl.help())->Show();
    return false;
  }

  if (arg1.empty() || arg2.empty())
  {
    log::status("empty arguments") << arg1 << "or" << arg2;
    return false;
  }

  const wex::tool tool(sed ? ID_TOOL_REPLACE : ID_TOOL_REPORT_FIND);

  if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
  {
    path::current(stc->path().data().parent_path());
  }

  find_replace_data::get()->set_regex(true);
  log::status(find_replace_string(false));

  wex::dir dir(
    path(arg1),
    data::dir()
      .file_spec(arg2)
      .type(arg3)
      .find_replace_data(find_replace_data::get())
      .vcs(m_vcs),
    activate_and_clear(tool));

  dir.find_files(tool);

  return true;
}

void wex::del::frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptible::is_running())
      {
        interruptible::end();
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
            {
              flags.set(data::dir::FILES);
            }
            if (config(p->text_addrecursive()).get(true))
            {
              flags.set(data::dir::RECURSIVE);
            }
            if (config(p->text_addfolders()).get(true))
            {
              flags.set(data::dir::DIRS);
            }

            p->add_items(
              config(p->text_infolder()).get_first_of(),
              config(p->text_addwhat()).get_first_of(),
              flags);
          }
          break;

        case id_find_in_files:
        case id_replace_in_files:
          find_in_files((wex::window_id)dialogid);
          break;

        default:
          log::trace("on_command_item_dialog") << event.GetId();
      }
      break;

    default:
      assert(0);
  }
}

void wex::del::frame::on_notebook(wxWindowID id, wxWindow* page)
{
  if (is_closing() || !IsShown())
  {
    return;
  }

  if (auto* stc = dynamic_cast<wex::stc*>(page); stc != nullptr)
  {
    show_ex_bar(!stc->is_visual() ? SHOW_BAR : HIDE_BAR_FOCUS_STC, stc);

    set_recent_file(stc->path());

    statustext_vcs(stc);

    // This is to take care that current dir follows page selection.
    // Which is convenient for git grep, ls etc. and opening from stc window.
    path::current(stc->path().data().parent_path());
  }
}

bool wex::del::frame::open_from_action(
  const std::string& file,
  const std::string& move_ext)
{
  // :e [+command] [file]
  if (auto text(file); !text.empty())
  {
    if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
    {
      wex::path::current(stc->path().data().parent_path());

      if (!marker_and_register_expansion(&stc->get_vi(), text))
      {
        return false;
      }
    }

    if (!shell_expansion(text))
    {
      return false;
    }

    std::string cmd;
    if (regex v("\\+([^ \t]+)* *(.*)"); v.match(text) > 1)
    {
      cmd  = v[0];
      text = v[1];
    }

    return open_files(
      this,
      to_vector_path(text).get(),
      data::control().command(cmd));
  }
  else
  {
    data::window data;
    data.style(wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR | wxFD_HEX_MODE)
      .allow_move_path_extension(move_ext);
    open_files_dialog(this, false, data::stc(data));
    return true;
  }
}

bool wex::del::frame::process_async_system(const process_data& data)
{
  if (m_process != nullptr)
  {
    if (m_process->is_running())
    {
      log::trace("escape") << data.exe() << "stops" << m_process->data().exe();
    }

    delete m_process;
  }

  m_process = new wex::process();

  return m_process->async_system(data);
}

void wex::del::frame::set_recent_file(const wex::path& path)
{
  wex::frame::set_recent_file(path);

  if (m_file_history_listview != nullptr && path.file_exists())
  {
    listitem(m_file_history_listview, path).insert(0);

    if (m_file_history_listview->GetItemCount() > 1)
    {
      for (auto i = m_file_history_listview->GetItemCount() - 1; i >= 1; i--)
      {
        if (listitem item(m_file_history_listview, i); item.path() == path)
        {
          item.erase();
        }
      }
    }
  }
}

void wex::del::frame::show_ex_bar(int action, syntax::stc* stc)
{
  if (action == SHOW_BAR || stc != nullptr)
  {
    if (action >= SHOW_BAR && stc != nullptr)
    {
      m_ex_commandline->set_stc(stc, ":");
    }
    else if (stc != nullptr)
    {
      m_ex_commandline->set_stc(stc);
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
      m_ex_commandline->stc() != nullptr)
    {
      m_ex_commandline->stc()->SetFocus();
    }
  }
}

void wex::del::frame::show_ex_message(const std::string& text)
{
  if (!is_ex(m_ex_commandline))
  {
    show_ex_bar();
  }

  log::status(text);
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
  else if (pane == "PaneLexer")
  {
    if (stc != nullptr && lexers_dialog(stc))
    {
      statustext(stc->get_lexer().display_lexer(), pane);
    }
  }
  else if (pane == "PaneFileType")
  {
    if (stc != nullptr)
    {
      stc->filetype_menu();
    }
  }
  else if (pane == "PaneMacro")
  {
    if (stc != nullptr)
    {
      stc->get_vi().get_macros().mode().transition("@", &stc->get_vi(), true);
    }
  }
  else if (pane == "PaneVCS")
  {
    if (!wex::vcs::empty())
    {
      auto* menu = new wex::menu(menu::menu_t_def().set(menu::IS_VISUAL));

      if (stc != nullptr)
      {
        if (menu->append({{path(stc->path().parent_path()), this}}))
        {
          PopupMenu(menu);
        }
      }
      else if (menu->append({{wex::path(), this}}))
      {
        PopupMenu(menu);
      }

      delete menu;
    }
  }
  else
  {
    wex::frame::statusbar_clicked(pane);
  }
}

void wex::del::frame::statusbar_clicked_right(const std::string& pane)
{
  if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); pane == "PaneInfo")
  {
    POPUPMENU_DO(is_shown_line_numbers, show_line_numbers);
  }
  else if (pane == "PaneFileType")
  {
    POPUPMENU_DO(GetViewEOL, show_whitespace);
  }
  else if (pane == "PaneMacro")
  {
    if (wex::ex::get_macros().path().file_exists())
    {
      open_file(
        wex::ex::get_macros().path(),
        wex::data::control().find(
          !get_statustext(pane).empty() ?
            " name=\"" + get_statustext(pane) + "\"" :
            std::string()));
    }
  }
  else if (pane == "PaneDBG" || pane == "PaneVCS")
  {
    auto match(get_statustext(pane));

    if (stc != nullptr)
    {
      match =
        (pane == "PaneVCS" ? wex::vcs({stc->path()}).entry().name() :
                             wex::debug(this).debug_entry().name());
    }

    open_file(wex::menus::path(), wex::data::control().find(match));
  }
  else
  {
    wex::frame::statusbar_clicked_right(pane);
  }
}

void wex::del::frame::statustext_vcs(factory::stc* stc)
{
  m_vcs->set(stc->path());

  if (const auto& text(m_vcs->get_branch()); !text.empty())
  {
    statustext(text, "PaneVCS");
  }
  else
  {
    statustext(m_vcs->name(), "PaneVCS");
  }
}

wex::syntax::stc* wex::del::frame::stc_entry_dialog_component()
{
  return entry_dialog()->get_stc();
}

int wex::del::frame::stc_entry_dialog_show(bool modal)
{
  return modal ? entry_dialog()->ShowModal() : entry_dialog()->Show();
}

std::string wex::del::frame::stc_entry_dialog_title() const
{
  return m_entry_dialog == nullptr ? std::string() :
                                     m_entry_dialog->GetTitle().ToStdString();
}

void wex::del::frame::stc_entry_dialog_title(const std::string& title)
{
  entry_dialog(title)->SetTitle(title);
}

void wex::del::frame::stc_entry_dialog_validator(const std::string& regex)
{
  entry_dialog()->set_validator(regex);
}

void wex::del::frame::use_file_history_list(listview* list)
{
  assert(list->data().type() == data::listview::HISTORY);

  m_file_history_listview = list;
  m_file_history_listview->Hide();

  // Add all (existing) items from file_history.
  for (size_t i = 0; i < file_history().size(); i++)
  {
    if (listitem item(m_file_history_listview, file_history()[i]);
        item.path().stat().is_ok())
    {
      item.insert();
    }
  }
}

void wex::del::frame::vcs_add_path(factory::link* l)
{
  if (wex::vcs v; v.use() && v.toplevel().dir_exists())
  {
    l->add_path(v.toplevel());
  }
}

bool wex::del::frame::vcs_annotate_commit(
  syntax::stc*       stc,
  int                line,
  const std::string& commit_id)
{
  if (commit_id.empty())
  {
    // this is not an error
    return false;
  }

  if (line > stc->get_line_count())
  {
    log("annotate commit line not present") << line;
    return false;
  }

  if (wex::vcs vcs{
        {!stc->get_data()->head_path().empty() ? stc->get_data()->head_path() :
                                                 stc->path()}};
      vcs.entry().log(stc->path(), commit_id))
  {
    stc->AnnotationSetText(
      line,
      lexer().make_comment(boost::algorithm::trim_copy(vcs.entry().std_out())));

    return true;
  }
  /* NOLINTNEXTLINE */
  else
  {
    log::trace("annotate commit failed") << vcs.name();
  }

  return false;
}

void wex::del::frame::vcs_append(wex::menu* menu, const menu_item* item) const
{
  if (!item->path().file_exists())
  {
    if (item->path().dir_exists())
    {
      append_submenu(item, menu);
    }
    else
    {
      wex::vcs vcs;

      if (vcs.set_entry_from_base(
            item->is_modal() ? wxTheApp->GetTopWindow() : nullptr))
      {
        auto* submenu = new wex::menu(menu->style());

        if (vcs.entry().build_menu(ID_EDIT_VCS_LOWEST + 1, submenu))
        {
          menu->append({{submenu, vcs.entry().name()}});
        }
      }
    }
  }
  else
  {
    append_submenu(item, menu);
  }
}

bool wex::del::frame::vcs_blame(syntax::stc* stc)
{
  if (wex::vcs vcs{{stc->path()}}; vcs.execute("blame " + stc->path().string()))
  {
    return vcs_blame_show(&vcs.entry(), stc);
  }

  return false;
}

bool wex::del::frame::vcs_blame_revision(
  syntax::stc*       stc,
  const std::string& renamed,
  const std::string& offset)
{
  blaming bl(stc, offset);

  if (!bl.execute(stc->path()))
  {
    return false;
  }

  data::stc data(*stc->get_data());
  data.control().line(stc->get_margin_text_click() + 1);
  data::window window(stc->get_data()->control().window());
  window.name(std::string());
  data.control().window(window);

  if (data.head_path().empty())
  {
    data.head_path(stc->path());
  }

  open_file_vcs(wex::path(path(bl.renamed())), bl.vcs().entry(), data);

  return true;
}

bool wex::del::frame::vcs_blame_show(vcs_entry* vcs, syntax::stc* stc)
{
  if (!vcs->get_blame().use() || vcs->std_out().empty())
  {
    log::debug("no blame (or no output)");
    return false;
  }

  log::trace("blame show") << vcs->name();

  const bool  is_empty(stc->GetTextLength() == 0);
  std::string prev("!@#$%");
  stc->SetWrapMode(wxSTC_WRAP_NONE);
  wex::blame* blame = &vcs->get_blame();
  bool        first = true;

  blame->line_no(-1);
  int ex_line_no = 0;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         vcs->std_out(),
         boost::char_separator<char>("\r\n")))
  {
    blame->parse(stc->path(), it);

    if (first)
    {
      stc->blame_margin(blame);
      first = false;
    }

    if (blame->info() != prev)
    {
      prev = blame->info();
    }
    else
    {
      blame->skip_info(true);
    }

    if (!stc->is_visual())
    {
      blame->line_no(ex_line_no++);
    }

    if (is_empty)
    {
      stc->add_text(blame->line_text() + "\n");
    }

    lexers::get()->apply_margin_text_style(stc, blame);
  }

  return true;
}

void wex::del::frame::vcs_destroy_dialog()
{
  vcs::destroy_dialog();
}

bool wex::del::frame::vcs_dir_exists(const path& p) const
{
  return vcs::dir_exists(p);
}

bool wex::del::frame::vcs_execute(
  int                           event_id,
  const std::vector<wex::path>& paths,
  const data::window&           data)

{
  return wex::vcs_execute(this, event_id, paths, data);
}

bool wex::del::frame::vcs_unified_diff(
  const vcs_entry*    entry,
  const unified_diff* diff)
{
  if (!diff->path_vcs().file_exists())
  {
    return false;
  }

  if (auto* stc = dynamic_cast<syntax::stc*>(open_file(diff->path_vcs()));
      stc != nullptr)
  {
    if (diff->range_from_count() > 0)
    {
      // deleted text, just a marker, and annotation with text
      stc->MarkerAdd(diff->range_from_start() - 1, m_marker_del.number());

      if (!diff->text_removed().empty())
      {
        stc->AnnotationSetText(
          diff->range_from_start() - 1,
          get_some_text(diff->text_removed()));
      }
    }

    if (diff->range_to_count() > 0)
    {
      if (!stc->set_indicator(
            m_indicator_add,
            stc->PositionFromLine(diff->range_to_start() - 1),
            stc->GetLineEndPosition(
              diff->range_to_start() - 2 + diff->range_to_count())))
      {
        log("vcs_unified_diff") << diff->path_vcs().string();
        return false;
      }
    }
  }

  return true;
}

bool wex::del::frame::vi_is_address(syntax::stc* stc, const std::string& text)
  const
{
  if (auto* wexstc = dynamic_cast<wex::stc*>(stc); wexstc != nullptr)
  {
    return wexstc->get_vi().is_address(text);
  }

  return false;
}
