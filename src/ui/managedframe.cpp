////////////////////////////////////////////////////////////////////////////////
// Name:      managed_frame.cpp
// Purpose:   Implementation of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wex/bind.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/macro-mode.h>
#include <wex/macros.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>
#include <wex/textctrl.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wx/panel.h>
#include <wx/stattext.h>

namespace wex
{
  bool is_ex(textctrl* tc)
  {
    return tc != nullptr && tc->ex() != nullptr &&
           tc->ex()->get_stc() != nullptr && !tc->ex()->get_stc()->is_visual();
  }
} // namespace wex

wex::managed_frame::managed_frame(size_t maxFiles, const data::window& data)
  : frame(data)
  , m_debug(new debug(this))
  , m_file_history(maxFiles, wxID_FILE1)
  , m_findbar(new toolbar(this))
  , m_optionsbar(new toolbar(this))
  , m_toolbar(new toolbar(this))
  , m_toggled_panes(
      {{{"FINDBAR", _("&Findbar")}, ID_VIEW_LOWEST + 1},
       {{"OPTIONSBAR", _("&Optionsbar")}, ID_VIEW_LOWEST + 2},
       {{"TOOLBAR", _("&Toolbar")}, ID_VIEW_LOWEST + 3},
       {{"PROCESS", _("&Process")}, ID_VIEW_LOWEST + 4}})
{
  m_manager.SetManagedWindow(this);

  add_toolbar_panes(
    {{m_toolbar, wxAuiPaneInfo().Name("TOOLBAR").Caption(_("Toolbar"))},
     {m_findbar, wxAuiPaneInfo().Name("FINDBAR").Caption(_("Findbar"))},
     {m_optionsbar,
      wxAuiPaneInfo().Name("OPTIONSBAR").Caption(_("Optionsbar"))},
     {create_ex_panel(), wxAuiPaneInfo().Name("VIBAR")}});

  show_ex_bar();

  Bind(wxEVT_AUI_PANE_CLOSE, [=, this](wxAuiManagerEvent& event) {
    auto* info = event.GetPane();
    info->BestSize(info->window->GetSize()).Fixed().Resizable();
    m_manager.Update();
    // If this pane is a toolbar pane, it might have a checkbox,
    // update that as well.
    m_optionsbar->set_checkbox(info->name, false);
  });

  Bind(wxEVT_CLOSE_WINDOW, [=, this](wxCloseEvent& event) {
    m_file_history.save();

    if (!m_perspective.empty())
    {
      wex::config(m_perspective).set(m_manager.SavePerspective().ToStdString());
    }

    event.Skip();
  });

  for (const auto& it : m_toggled_panes)
  {
    Bind(
      wxEVT_UPDATE_UI,
      [=, this](wxUpdateUIEvent& event) {
        event.Check(m_manager.GetPane(it.first.first).IsShown());
      },
      it.second);

    Bind(
      wxEVT_MENU,
      [=, this](wxCommandEvent& event) {
        pane_toggle(it.first.first);
      },
      it.second);
  }

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event) {
      on_menu_history(
        m_file_history,
        event.GetId() - m_file_history.get_base_id());
    },
    m_file_history.get_base_id(),
    m_file_history.get_base_id() + m_file_history.get_max_files());

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
        m_file_history.clear();
      },
      ID_CLEAR_FILES},
     {[=, this](wxCommandEvent& event) {
        if (auto* stc = get_stc(); stc != nullptr)
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
}

wex::managed_frame::~managed_frame()
{
  delete m_debug;

  m_manager.UnInit();
}

bool wex::managed_frame::add_toolbar_panes(const panes_t& panes)
{
  panes_t pns;

  for (const auto& it : panes)
  {
    wxAuiPaneInfo pane(it.second);

    pane.LeftDockable(false).RightDockable(false);

    // If the toolbar has a caption.
    if (!pane.caption.empty())
    {
#ifndef __WXOSX__
      pane.Top().ToolbarPane().MinSize(-1, 30);
#else
      pane.Bottom().ToolbarPane().MinSize(-1, 30);
#endif

      // Initially hide special bars.
      if (pane.name == "FINDBAR" || pane.name == "OPTIONSBAR")
      {
        pane.Hide();
      }
    }
    // otherwise (vi) fixed at the bottom and initially hidden.
    else
    {
      pane.Bottom()
        .CloseButton(false)
        .Hide()
        .DockFixed(true)
        .Movable(false)
        .Resizable()
        .Row(10)
        .CaptionVisible(false);
    }

    pns.push_back({it.first, pane});
  }

  return pane_add(pns);
}

bool wex::managed_frame::allow_close(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  set_find_focus(nullptr);
  m_textctrl->set_ex(nullptr, std::string());

  return true;
}

wxPanel* wex::managed_frame::create_ex_panel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex textctrl for getting user input.
  auto* panel = new wxPanel(this);
  auto* text  = new wxStaticText(panel, wxID_ANY, " ");
  m_textctrl  = new textctrl(this, text, data::window().parent(panel));

  auto* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Center());
  sizer->Add(m_textctrl->control(), wxSizerFlags().Expand());

  panel->SetSizerAndFit(sizer);

  return panel;
}

void wex::managed_frame::on_menu_history(
  const class file_history& history,
  size_t                    index,
  data::stc::window_t       flags)
{
  if (const auto& file(history.get_history_file(index)); !file.empty())
  {
    open_file(file, data::stc().flags(flags));
  }
}

void wex::managed_frame::on_notebook(wxWindowID id, wxWindow* page)
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

wex::stc* wex::managed_frame::open_file(const path& file, const data::stc& data)
{
  if (auto* stc = frame::open_file(file, data); stc != nullptr)
  {
    set_recent_file(file);
    return stc;
  }

  return nullptr;
}

bool wex::managed_frame::pane_add(
  const panes_t&     panes,
  const std::string& perspective)
{
  for (const auto& it : panes)
  {
    if (!m_manager.AddPane(it.first, it.second))
    {
      return false;
    }
  }

  if (!perspective.empty())
  {
    m_perspective = "perspective." + perspective;

    if (const auto& val(wex::config(m_perspective).get()); !val.empty())
    {
      m_manager.LoadPerspective(val);
    }
  }

  m_manager.Update();

  return true;
}

wxWindow* wex::managed_frame::pane_get(const std::string& pane)
{
  return m_manager.GetPane(pane).window;
}

bool wex::managed_frame::pane_maximize(const std::string& pane)
{
  if (auto& info = m_manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else
  {
    info.Maximize();
    m_manager.Update();
    return true;
  }
}

bool wex::managed_frame::pane_restore(const std::string& pane)
{
  if (auto& info = m_manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else
  {
    info.Restore();
    m_manager.Update();
    return true;
  }
}

bool wex::managed_frame::pane_set(
  const std::string&   pane,
  const wxAuiPaneInfo& info)
{
  if (auto& current = m_manager.GetPane(pane); !current.IsOk())
  {
    return false;
  }
  else
  {
    if (info.best_size != wxDefaultSize)
    {
      current.BestSize(info.best_size);
    }

    m_manager.Update();
    return true;
  }
}

bool wex::managed_frame::pane_show(const std::string& pane, bool show)
{
  if (auto& info = m_manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else
  {
    show ? info.Show() : info.Hide();

    // ignore result, e.g. VIBAR
    m_optionsbar->set_checkbox(pane, show);

    m_manager.Update();
    return true;
  }
}

size_t wex::managed_frame::panes() const
{
  return const_cast<managed_frame*>(this)->m_manager.GetAllPanes().GetCount();
}

void wex::managed_frame::print_ex(ex* ex, const std::string& text)
{
  ex->print(text);
}

void wex::managed_frame::set_recent_file(const path& path)
{
  m_file_history.append(path);
}

void wex::managed_frame::show_ex_bar(int action, ex* ex)
{
  if (action == SHOW_BAR || ex != nullptr)
  {
    if (action >= SHOW_BAR)
    {
      m_textctrl->set_ex(ex, ":");
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
      m_textctrl->ex() != nullptr && m_textctrl->ex()->get_stc() != nullptr)
    {
      m_textctrl->ex()->get_stc()->SetFocus();
    }
  }
}

bool wex::managed_frame::show_ex_command(ex* ex, const std::string& command)
{
  return pane_show("VIBAR") && m_textctrl->set_ex(ex, command);
}

bool wex::managed_frame::show_ex_input(ex* ex, char cmd)
{
  return pane_show("VIBAR") && m_textctrl->set_ex(ex, cmd);
}

void wex::managed_frame::show_ex_message(const std::string& text)
{
  if (!is_ex(m_textctrl))
  {
    show_ex_bar();
  }

  statustext(text, std::string());
}

void wex::managed_frame::statusbar_clicked(const std::string& pane)
{
  if (pane == "PaneDBG")
  {
    if (get_debug()->show_dialog(this))
    {
      statustext(get_debug()->debug_entry().name(), pane);
    }
  }
  else
  {
    frame::statusbar_clicked(pane);
  }
}

void wex::managed_frame::show_process(bool show)
{
  pane_show("PROCESS", show);
}

void wex::managed_frame::statusbar_clicked_right(const std::string& pane)
{
  if (pane == "PaneInfo")
  {
    if (auto* stc = get_stc(); stc != nullptr)
    {
      PopupMenu(new wex::menu(
        {{wxWindow::NewControlId(),
          stc->is_shown_line_numbers() ? "&Hide" : "&Show",
          data::menu().action([=, this](wxCommandEvent&) {
            stc->show_line_numbers(!stc->is_shown_line_numbers());
          })}}));
    }
  }
  else if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    std::string match;

    if (pane == "PaneLexer")
    {
      if (auto* stc = get_stc(); stc != nullptr)
      {
        if (
          !stc->get_lexer().scintilla_lexer().empty() &&
          stc->get_lexer().scintilla_lexer() ==
            stc->get_lexer().display_lexer())
        {
          match =
            "lexer *name *= *\"" + stc->get_lexer().scintilla_lexer() + "\"";
        }
        else if (!stc->get_lexer().display_lexer().empty())
        {
          match = "display *= *\"" + stc->get_lexer().display_lexer() + "\"";
        }
        else
        {
          return;
        }
      }
    }
    else
    {
      if (wex::lexers::get()->theme().empty())
      {
        return;
      }

      match = "theme *name *= *\"" + wex::lexers::get()->theme() + "\"";
    }

    open_file(
      wex::lexers::get()->get_filename(),
      wex::data::control().find(
        match,
        wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX));
  }
  else if (pane == "PaneMacro")
  {
    if (wex::ex::get_macros().get_filename().file_exists())
    {
      open_file(
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

    if (auto* stc = get_stc(); stc != nullptr)
    {
      match =
        (pane == "PaneVCS" ?
           wex::vcs({stc->get_filename().string()}).entry().name() :
           wex::debug(this).debug_entry().name());
    }

    open_file(wex::menus::get_filename(), wex::data::control().find(match));
  }
  else if (pane == "PaneText")
  {
    wex::config::save();
    open_file(wex::config::file());
  }
  else
  {
    frame::statusbar_clicked_right(pane);
  }
}

void wex::managed_frame::sync_all()
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->sync(config("AllowSync").get(true));
  }
}

void wex::managed_frame::sync_close_all(wxWindowID id)
{
  set_find_focus(nullptr);

  show_ex_bar(SHOW_BAR_SYNC_CLOSE_ALL);
}
