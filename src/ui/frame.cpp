////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/tostring.h>
#include <wex/common/util.h>
#include <wex/core/cmdline.h>
#include <wex/core/config.h>
#include <wex/core/file.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/factory/listview.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/syntax/stc.h>
#include <wex/ui/ex-commandline.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/grid.h>
#include <wex/ui/toolbar.h>

#include <wx/app.h>
#include <wx/fdrepdlg.h>
#include <wx/panel.h>
#include <wx/stattext.h>

#define FIND_REPLACE(text, dlg)                                          \
  {                                                                      \
    if (m_find_replace_dialog != nullptr)                                \
    {                                                                    \
      m_find_replace_dialog->Destroy();                                  \
    }                                                                    \
                                                                         \
    auto* win = wxWindow::FindFocus();                                   \
                                                                         \
    if (auto* cl = dynamic_cast<wex::syntax::stc*>(win); cl != nullptr)  \
    {                                                                    \
      m_find_focus = cl;                                                 \
    }                                                                    \
    /* NOLINTNEXTLINE */                                                 \
    else                                                                 \
    {                                                                    \
      if (auto* cl = dynamic_cast<wex::factory::listview*>(win);         \
          cl != nullptr)                                                 \
      {                                                                  \
        m_find_focus = cl;                                               \
      }                                                                  \
      /* NOLINTNEXTLINE */                                               \
      else                                                               \
      {                                                                  \
        if (auto* grid = dynamic_cast<wex::grid*>(win); grid != nullptr) \
        {                                                                \
          m_find_focus = grid;                                           \
        }                                                                \
      }                                                                  \
    }                                                                    \
                                                                         \
    if (auto* stc = get_stc(); stc != nullptr)                           \
    {                                                                    \
      stc->get_find_string();                                            \
    }                                                                    \
                                                                         \
    m_find_replace_dialog = new wxFindReplaceDialog(                     \
      this,                                                              \
      wex::find_replace_data::get()->data(),                             \
      text,                                                              \
      dlg);                                                              \
    m_find_replace_dialog->Show();                                       \
  };

namespace wex
{
class file_droptarget : public wxFileDropTarget
{
public:
  explicit file_droptarget(factory::frame* frame)
    : m_frame(frame)
  {
    ;
  };

  bool
  OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override
  {
    open_files(m_frame, to_vector_path(filenames).get());
    return true;
  }

private:
  factory::frame* m_frame;
};

wex::config::ints_t win_data;

bool win_shown = false;

/* NOLINTNEXTLINE */
const std::string win_frame = "window.frame", win_max = "window.maximized";
} // namespace wex

wex::frame::frame(size_t maxFiles, const data::window& data)
  : m_file_history(maxFiles, wxID_FILE1)
  , m_toggled_panes(
      {{{"FINDBAR", _("&Findbar")}, ID_VIEW_LOWEST + 1},
       {{"OPTIONSBAR", _("&Optionsbar")}, ID_VIEW_LOWEST + 2},
       {{"TOOLBAR", _("&Toolbar")}, ID_VIEW_LOWEST + 3},
       {{"PROCESS", _("&Process")}, ID_VIEW_LOWEST + 4}})
{
  Create(
    data.parent(),
    data.id(),
    data.title().empty() ? std::string(wxTheApp->GetAppDisplayName()) :
                           data.title(),
    data.pos(),
    data.size(),
    data.style() == data::NUMBER_NOT_SET ? wxDEFAULT_FRAME_STYLE : data.style(),
    data.name().empty() ? "frame" : data.name());

  m_findbar    = new toolbar(this);
  m_optionsbar = new toolbar(this);
  m_toolbar    = new toolbar(this);

  m_manager.SetManagedWindow(this);

  win_data = config(win_frame).get(config::ints_t{
    data.size().GetWidth(),
    data.size().GetHeight(),
    data.pos().x,
    data.pos().y});

  printing::get()->get_html_printer()->SetParentWindow(this);

  Bind(
    wxEVT_FIND,
    [=, this](wxFindDialogEvent& event)
    {
      if (m_find_focus != nullptr)
      {
        wxPostEvent(m_find_focus, event);
      }
    });
  Bind(
    wxEVT_FIND_NEXT,
    [=, this](wxFindDialogEvent& event)
    {
      if (m_find_focus != nullptr)
      {
        wxPostEvent(m_find_focus, event);
      }
    });
  Bind(
    wxEVT_FIND_REPLACE,
    [=, this](wxFindDialogEvent& event)
    {
      if (m_find_focus != nullptr)
      {
        wxPostEvent(m_find_focus, event);
      }
    });
  Bind(
    wxEVT_FIND_REPLACE_ALL,
    [=, this](wxFindDialogEvent& event)
    {
      if (m_find_focus != nullptr)
      {
        wxPostEvent(m_find_focus, event);
      }
    });

  Bind(
    wxEVT_FIND_CLOSE,
    [=, this](wxFindDialogEvent& event)
    {
      assert(m_find_replace_dialog != nullptr);
      // Hiding instead of destroying, does not
      // show the dialog next time.
      m_find_replace_dialog->Destroy();
      m_find_replace_dialog = nullptr;
    });

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        if (GetStatusBar() != nullptr)
        {
          GetStatusBar()->Show(!GetStatusBar()->IsShown());
          SendSizeEvent();
        }
      },
      ID_VIEW_STATUSBAR},
     {[=, this](wxCommandEvent& event)
      {
        FIND_REPLACE(_("Find"), 0);
      },
      wxID_FIND},
     {[=, this](wxCommandEvent& event)
      {
        FIND_REPLACE(_("Replace"), wxFR_REPLACEDIALOG);
      },
      wxID_REPLACE}});

  bind(this).ui(
    {{[=, this](wxUpdateUIEvent& event)
      {
        (GetStatusBar() != nullptr ? event.Check(GetStatusBar()->IsShown()) :
                                     event.Check(false));
      },
      ID_VIEW_STATUSBAR},
     {[=, this](wxUpdateUIEvent& event)
      {
        (GetMenuBar() != nullptr ? event.Check(GetMenuBar()->IsShown()) :
                                   event.Check(false));
      },
      ID_VIEW_MENUBAR}});

  Bind(
    wxEVT_CLOSE_WINDOW,
    [=, this](wxCloseEvent& event)
    {
      m_file_history.save();
      m_ex_commandline->on_exit();

      delete find_replace_data::set(nullptr);

      if (!m_perspective.empty())
      {
        wex::config(m_perspective)
          .set(m_manager.SavePerspective().ToStdString());
      }

      if (IsMaximized())
      {
        config(win_max).set(true);
      }
      else
      {
        config(win_max).set(false);
        config(win_frame).set(config::ints_t{
          GetSize().GetWidth(),
          GetSize().GetHeight(),
          GetPosition().x,
          GetPosition().y});
      }

      m_is_closing = true;

      event.Skip();
    });

  SetDropTarget(new file_droptarget(this));

  add_toolbar_panes(
    {{m_toolbar, wxAuiPaneInfo().Name("TOOLBAR").Caption(_("Toolbar"))},
     {m_findbar, wxAuiPaneInfo().Name("FINDBAR").Caption(_("Findbar"))},
     {m_optionsbar,
      wxAuiPaneInfo().Name("OPTIONSBAR").Caption(_("Optionsbar"))},
     {create_ex_panel(), wxAuiPaneInfo().Name("VIBAR")}});

  show_ex_bar();

  Bind(
    wxEVT_AUI_PANE_CLOSE,
    [=, this](wxAuiManagerEvent& event)
    {
      auto* info = event.GetPane();
      info->BestSize(info->window->GetSize()).Fixed().Resizable();
      m_manager.Update();
      // If this pane is a toolbar pane, it might have a checkbox,
      // update that as well.
      m_optionsbar->set_checkbox(info->name, false);
    });

  for (const auto& it : m_toggled_panes)
  {
    Bind(
      wxEVT_UPDATE_UI,
      [=, this](wxUpdateUIEvent& event)
      {
        event.Check(m_manager.GetPane(it.first.first).IsShown());
      },
      it.second);

    Bind(
      wxEVT_MENU,
      [=, this](wxCommandEvent& event)
      {
        pane_toggle(it.first.first);
      },
      it.second);
  }

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event)
    {
      on_menu_history(
        m_file_history,
        event.GetId() - m_file_history.get_base_id());
    },
    m_file_history.get_base_id(),
    m_file_history.get_base_id() + m_file_history.get_max_files());

  if (cmdline::is_output())
  {
    auto* logger = new wxLogStream(&std::cout);
    wxLog::SetActiveTarget(logger);
  }
  else if (!cmdline::get_output().empty())
  {
    m_ofs = new std::ofstream(
      cmdline::get_output(),
      std::ios_base::out | std::ios_base::app);

    auto* logger = new wxLogStream(m_ofs);
    wxLog::SetActiveTarget(logger);
  }
}

wex::frame::~frame()
{
  m_manager.UnInit();

  if (m_ofs != nullptr)
  {
    delete m_ofs;
  }

  delete m_ex_commandline;
}

bool wex::frame::add_toolbar_panes(const panes_t& panes)
{
  panes_t pns;

  for (const auto& it : panes)
  {
    wxAuiPaneInfo pane(it.second);

    pane.LeftDockable(false).RightDockable(false);

    // If the toolbar has a caption.
    if (!pane.caption.empty())
    {
      if (pane.name == "FINDBAR")
      {
        pane.Top()
          .CloseButton(false)
          .DockFixed(true)
          .Movable(false)
          .CaptionVisible(false);
      }
      else
      {
        pane.Top().ToolbarPane().MinSize(-1, 30);
      }

      // Initially hide special bars.
      if (pane.name == "FINDBAR" || pane.name == "OPTIONSBAR")
      {
        pane.Hide();
      }
    }
    // Otherwise, (vi) fixed at the bottom and initially hidden.
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

bool wex::frame::allow_browse_backward() const
{
  return m_browse_index < m_file_history.size() && m_file_history.size() > 1 &&
         m_browse_index != 0;
}

bool wex::frame::allow_browse_forward() const
{
  return m_browse_index < m_file_history.size() - 1 &&
         m_file_history.size() > 1;
}

bool wex::frame::allow_close(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  set_find_focus(nullptr);
  m_ex_commandline->set_stc(nullptr, std::string());

  return true;
}

bool wex::frame::browse(wxCommandEvent& event)
{
  if (m_file_history.size() <= 1)
  {
    return false;
  }

  switch (event.GetId())
  {
    case wxID_BACKWARD:
      if (m_browse_index > 0)
      {
        m_browse_index--;
      }
      else
      {
        if (m_browse_index > m_file_history.size() - 1)
        {
          m_browse_index = m_file_history.size() - 1;
        }

        return false;
      }
      break;

    case wxID_FORWARD:
      if (m_browse_index < m_file_history.size() - 1)
      {
        m_browse_index++;
      }
      else
      {
        if (m_browse_index > m_file_history.size() - 1)
        {
          m_browse_index = m_file_history.size() - 1;
        }

        return false;
      }
      break;

    default:
      assert(0);
  }

  open_file_same_page(m_file_history[m_browse_index]);

  return true;
}

wxPanel* wex::frame::create_ex_panel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex ex_commandline for getting user input.
  auto* panel = new wxPanel(this);
  auto* text  = new wxStaticText(panel, wxID_ANY, "  ");
  m_ex_commandline =
    new ex_commandline(this, text, data::window().parent(panel));

  auto* sizer = new wxFlexGridSizer(3);
  sizer->AddGrowableCol(2);
  sizer->Add(text, wxSizerFlags().Top()); // similar to ex_commandline->control
  sizer->AddSpacer(2);
  sizer->Add(m_ex_commandline->control(), wxSizerFlags().Expand());

  panel->SetSizerAndFit(sizer);

  return panel;
}

bool wex::frame::Destroy()
{
  if (m_find_replace_dialog != nullptr)
  {
    m_find_replace_dialog->Destroy();
  }

  return factory::frame::Destroy();
}

std::string wex::frame::get_statustext(const std::string& pane) const
{
  return (
    m_statusbar == nullptr ? std::string() : m_statusbar->get_statustext(pane));
}

wxStatusBar* wex::frame::OnCreateStatusBar(
  int             number,
  long            style,
  wxWindowID      id,
  const wxString& name)
{
  m_statusbar = new wex::statusbar(
    this,
    wex::data::window().id(id).style(style).name(name));

  m_statusbar->SetFieldsCount(number);

  return m_statusbar;
}

void wex::frame::on_menu_history(
  const class file_history& history,
  size_t                    index,
  data::stc::window_t       flags)
{
  if (const auto& file(history[index]); !file.empty())
  {
    open_file(file, data::stc().flags(flags));
  }
}

wex::factory::stc*
wex::frame::open_file(const path& file, const data::stc& data)
{
  if (auto* stc = factory::frame::open_file(file, data); stc != nullptr)
  {
    set_recent_file(file);
    return stc;
  }

  return nullptr;
}

bool wex::frame::output(const std::string& text) const
{
  provide_output(text);
  return factory::frame::output(text);
}

void wex::frame::provide_output(const std::string& text) const
{
  if (cmdline::is_output())
  {
    std::cout << text;
  }

  if (!cmdline::get_output().empty())
  {
    wex::file(
      wex::path(cmdline::get_output()),
      std::ios_base::out | std::ios_base::app)
      .write(text);
  }
}

const std::string wex::frame::pane_add(wxWindow* pane)
{
  static int no = 0;

  const auto& info(
    panes() == 5 ? wxAuiPaneInfo().Center() : wxAuiPaneInfo().Bottom());

  const std::string name("PANE " + std::to_string(no++));

  pane_add(
    {{pane, wxAuiPaneInfo(info).Name(name).MinSize(250, 30).Caption(name)}});

  return name;
}

bool wex::frame::pane_add(const panes_t& panes, const std::string& perspective)
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

  pane_show("FINDBAR", false);

  // This should not be necessary, but when exiting with a shown findbar,
  // it reappears too large.
  m_manager.Update();

  return true;
}

wxWindow* wex::frame::pane_get(const std::string& pane)
{
  return m_manager.GetPane(pane).window;
}

bool wex::frame::pane_maximize(const std::string& pane)
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

bool wex::frame::pane_restore(const std::string& pane)
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

bool wex::frame::pane_set(const std::string& pane, const wxAuiPaneInfo& info)
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

bool wex::frame::pane_show(const std::string& pane, bool show)
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

size_t wex::frame::panes() const
{
  return const_cast<frame*>(this)->m_manager.GetAllPanes().GetCount();
}

void wex::frame::record(const std::string& command)
{
  if (cmdline::is_echo())
  {
    std::cout << command << "\n";
  }

  if (!cmdline::get_scriptout().empty())
  {
    wex::file(
      wex::path(cmdline::get_scriptout()),
      std::ios_base::out | std::ios_base::app)
      .write(command + "\n");
  }
}

void wex::frame::set_recent_file(const path& path)
{
  m_file_history.append(path);
}

wex::statusbar* wex::frame::setup_statusbar(
  const std::vector<statusbar_pane>& panes,
  long                               style,
  const std::string&                 name)
{
  return statusbar::setup(this, panes, style, name);
}

bool wex::frame::statustext(const std::string& text, const std::string& pane)
  const
{
  if (pane.empty())
  {
    provide_output(text);
  }

  return (
    m_is_closing || m_statusbar == nullptr ?
      false :
      m_statusbar->set_statustext(text, pane));
}

bool wex::frame::show_ex_command(syntax::stc* stc, const std::string& command)
{
  return pane_show("VIBAR") && m_ex_commandline->set_stc(stc, command);
}

bool wex::frame::show_ex_input(syntax::stc* stc, char cmd)
{
  return pane_show("VIBAR") && m_ex_commandline->set_stc(stc, cmd);
}

void wex::frame::statusbar_clicked_right(const std::string& pane)
{
  if (pane == "PaneLexer" || pane == "PaneTheme")
  {
    std::string match;

    if (pane == "PaneLexer")
    {
      if (auto* stc = dynamic_cast<syntax::stc*>(get_stc()); stc != nullptr)
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
      wex::lexers::get()->path(),
      wex::data::control().find(
        match,
        wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX));
  }
  else if (pane == "PaneText")
  {
    wex::config::save();
    open_file(wex::config::path());
  }
  else
  {
    factory::frame::statusbar_clicked_right(pane);
  }
}

void wex::frame::show_process(bool show)
{
  pane_show("PROCESS", show);
}

bool wex::frame::Show(bool show)
{
  if (show && !win_shown)
  {
    SetSize(win_data[0], win_data[1]);
    SetPosition(wxPoint(win_data[2], win_data[3]));

    if (config(win_max).get(false))
    {
      Maximize();
    }

    win_shown = true;
  }

  return wxFrame::Show(show);
}

void wex::frame::sync_close_all(wxWindowID id)
{
  set_find_focus(nullptr);

  show_ex_bar(SHOW_BAR_SYNC_CLOSE_ALL);
}
