////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wex/frame.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/grid.h>
#include <wex/lexers.h>
#include <wex/listview.h>
#include <wex/path.h>
#include <wex/printing.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/tostring.h>
#include <wex/vcsentry.h>

#define wxCAST_TO(classname)                                 \
  if (m_find_focus != nullptr && m_find_focus->IsShown())    \
  {                                                          \
    if (classname* win = dynamic_cast<classname*>(m_find_focus); \
      win != nullptr)                                        \
    {                                                        \
      return win;                                            \
    }                                                        \
  }                                                          \
                                                             \
  wxWindow* win = wxWindow::FindFocus();                     \
  classname* cl = dynamic_cast<classname*>(win);             \
  return cl;
  
#define FIND_REPLACE( text, dlg)                             \
{                                                            \
  if (m_find_replace_dialog != nullptr)                      \
  {                                                          \
    m_find_replace_dialog->Destroy();                        \
  }                                                          \
                                                             \
  auto* win = wxWindow::FindFocus();                         \
                                                             \
  if (auto* cl = dynamic_cast<wex::stc*>(win);               \
    cl != nullptr)                                           \
  {                                                          \
    m_find_focus = cl;                                       \
  }                                                          \
  else                                                       \
  {                                                          \
    if (auto* cl = dynamic_cast<wex::listview*>(win);        \
      cl != nullptr)                                         \
    {                                                        \
      m_find_focus = cl;                                     \
    }                                                        \
    else                                                     \
    {                                                        \
      if (auto* grid = dynamic_cast<wex::grid*>(win);        \
        grid != nullptr)                                     \
      {                                                      \
        m_find_focus = grid;                                 \
      }                                                      \
    }                                                        \
  }                                                          \
                                                             \
  if (auto* stc = get_stc(); stc != nullptr)                 \
  {                                                          \
    stc->get_find_string();                                  \
  }                                                          \
                                                             \
  m_find_replace_dialog = new wxFindReplaceDialog(           \
    this, wex::find_replace_data::get()->data(), text, dlg); \
  m_find_replace_dialog->Show();                             \
};                                                           \
  
namespace wex
{
  class file_droptarget : public wxFileDropTarget
  {
  public:
    explicit file_droptarget(frame* frame) 
      : m_frame(frame){;};

    bool OnDropFiles(wxCoord x, wxCoord y, 
      const wxArrayString& filenames) override {
        open_files(m_frame, to_vector_path(filenames).get());
        return true;}
  private:
    frame* m_frame;
  };
  
  const std::string 
    win_frame = "window.frame", 
    win_max = "window.maximized";
  
  std::vector<int> win_data;
  
  bool win_shown = false;
};

wex::frame::frame(const window_data& data)
  : wxFrame(
      data.parent(), 
      data.id(), 
      data.title().empty() ? 
        std::string(wxTheApp->GetAppDisplayName()): data.title(), 
      data.pos(), 
      data.size(), 
      data.style() == DATA_NUMBER_NOT_SET ? 
        wxDEFAULT_FRAME_STYLE: data.style(), 
      data.name().empty() ? "frame": data.name())
{
  SetDropTarget(new file_droptarget(this));
  
  win_data = config(win_frame).get(std::vector<int> {
    data.size().GetWidth(), 
    data.size().GetHeight(), 
    data.pos().x, 
    data.pos().y});
  
  wxAcceleratorEntry entries[4];
  entries[0].Set(wxACCEL_NORMAL, WXK_F5, wxID_FIND);
  entries[1].Set(wxACCEL_NORMAL, WXK_F6, wxID_REPLACE);
  entries[2].Set(wxACCEL_CTRL, (int)'I', ID_VIEW_MENUBAR);
  entries[3].Set(wxACCEL_CTRL, (int)'T', ID_VIEW_TITLEBAR);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  printing::get()->get_html_printer()->SetParentWindow(this);
#endif

  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    if (m_find_focus != nullptr) wxPostEvent(m_find_focus, event);});
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    if (m_find_focus != nullptr) wxPostEvent(m_find_focus, event);});
  Bind(wxEVT_FIND_REPLACE, [=](wxFindDialogEvent& event) {
    if (m_find_focus != nullptr) wxPostEvent(m_find_focus, event);});
  Bind(wxEVT_FIND_REPLACE_ALL, [=](wxFindDialogEvent& event) {
  if (m_find_focus != nullptr) wxPostEvent(m_find_focus, event);});

  Bind(wxEVT_FIND_CLOSE, [=](wxFindDialogEvent& event) {
    assert(m_find_replace_dialog != nullptr);
    // Hiding instead of destroying, does not 
    // show the dialog next time.
    m_find_replace_dialog->Destroy();
    m_find_replace_dialog = nullptr;});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetStatusBar() != nullptr)
    {
      GetStatusBar()->Show(!GetStatusBar()->IsShown());
      SendSizeEvent();
    }}, ID_VIEW_STATUSBAR);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    (GetStatusBar() != nullptr ? 
       event.Check(GetStatusBar()->IsShown()): event.Check(false));},
    ID_VIEW_STATUSBAR);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    if (auto* lv = get_listview(); lv != nullptr && lv->HasFocus())
    {
      update_statusbar(lv);
    }}, ID_UPDATE_STATUS_BAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    FIND_REPLACE(_("Find"), 0 );}, wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    FIND_REPLACE( _("Replace") , wxFR_REPLACEDIALOG );}, wxID_REPLACE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_is_command = true;

    // :e [+command] [file]
    if (!event.GetString().empty())
    {
      std::string text(event.GetString());

      if (auto* stc = get_stc(); stc != nullptr)
      {
        wex::path::current(stc->get_filename().get_path());
        if (!marker_and_register_expansion(&stc->get_vi(), text)) return;
      }

      if (!shell_expansion(text)) return;

      std::string cmd;
      if (std::vector <std::string> v; match("\\+([^ \t]+)* *(.*)", text, v) > 1)
      {
        cmd = v[0];
        text = v[1];
      }

      open_files(this, to_vector_path(text).get(), control_data().command(cmd));
    }
    else
    {
      open_files_dialog(this);
    }}, wxID_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    SetMenuBar(GetMenuBar() != nullptr ? nullptr: m_menubar);}, ID_VIEW_MENUBAR);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    SetWindowStyleFlag(!(GetWindowStyleFlag() & wxCAPTION) ? 
      wxDEFAULT_FRAME_STYLE:
      GetWindowStyleFlag() & ~wxCAPTION);
    Refresh();}, ID_VIEW_TITLEBAR);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    (GetMenuBar() != nullptr ? 
       event.Check(GetMenuBar()->IsShown()): 
       event.Check(false));},
    ID_VIEW_MENUBAR);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    if (IsMaximized())
    {
      config(win_max).set(true);
    }
    else
    {
      config(win_max).set(false);
      config(win_frame).set(std::vector<int> {
        GetSize().GetWidth(), 
        GetSize().GetHeight(), 
        GetPosition().x, 
        GetPosition().y});
    }

    m_is_closing = true;
    event.Skip(); });
}

wex::frame::~frame()
{
  if (m_find_replace_dialog != nullptr)
  {
    m_find_replace_dialog->Destroy();
  }
  
  config("show.MenuBar").set(
    GetMenuBar() != nullptr && GetMenuBar()->IsShown());
}

wex::grid* wex::frame::get_grid()
{
  wxCAST_TO(wex::grid);
}

wex::listview* wex::frame::get_listview()
{
  wxCAST_TO(wex::listview);
}

std::string wex::frame::get_statustext(const std::string& pane)
{
  return (m_statusbar == nullptr ? 
    std::string(): 
    m_statusbar->get_statustext(pane));
}

wex::stc* wex::frame::get_stc()
{
  wxCAST_TO(wex::stc);
}
  
bool wex::frame::is_open(const wex::path& filename)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    return stc->get_filename() == filename;
  }
  
  return false;
}
  
wxStatusBar* wex::frame::OnCreateStatusBar(
  int number,
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_statusbar = new wex::statusbar(this, 
    wex::window_data().id(id).style(style).name(name));
  m_statusbar->SetFieldsCount(number);
  return m_statusbar;
}

wex::stc* wex::frame::open_file(
  const wex::path& filename,
  const wex::stc_data& data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->open(filename, data);
    return stc;
  }

  return nullptr;
}

wex::stc* wex::frame::open_file(
  const wex::path& filename,
  const vcs_entry& vcs,
  const stc_data& data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->set_text(vcs.get_stdout());
    vcs_command_stc(vcs.get_command(), filename.lexer(), stc);
    return stc;
  }

  return nullptr;
}

wex::stc* wex::frame::open_file(
  const path& filename,
  const std::string& text,
  const stc_data& data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->set_text(text);
    return stc;
  }

  return nullptr;
}
    
void wex::frame::SetMenuBar(wxMenuBar* bar)
{
  if (bar != nullptr)
  {
    m_menubar = bar;
  }
  
  wxFrame::SetMenuBar(
   !m_is_command && !config("show.MenuBar").get(true) ? 
      nullptr: bar);
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
  
void wex::frame::statusbar_clicked(const std::string& pane)
{
  if (auto* stc = get_stc(); pane == "PaneInfo")
  {
    if (stc != nullptr) 
    {
      wxPostEvent(stc, wxCommandEvent(wxEVT_MENU, wxID_JUMP_TO));
    }
    else
    {
      if (auto* lv = get_listview();
        lv != nullptr) wxPostEvent(lv, wxCommandEvent(wxEVT_MENU, wxID_JUMP_TO));
    }
  }
  else if (pane == "PaneLexer")
  {
    if (stc != nullptr) lexers::get()->show_dialog(stc);
  }
  else if (pane == "PaneFileType")
  {
    if (stc != nullptr) stc->filetype_menu();
  }
  else
  {
    // Clicking on another field, do nothing. 
  }
}

bool wex::frame::statustext(const std::string& text, const std::string& pane)
{
  return (m_is_closing || m_statusbar == nullptr ? 
    false: 
    m_statusbar->set_statustext(text, pane));
}

bool wex::frame::update_statusbar(const wxListView* lv)
{
  if (!m_is_closing && lv->IsShown())
  {
    const auto text = std::to_string(lv->GetItemCount()) + 
      (lv->GetSelectedItemCount() > 0 ? 
         "," + std::to_string(lv->GetSelectedItemCount()):
         std::string());
      
    return statustext(text, "PaneInfo");
  }
  
  return false;
}

// Do not make it const, too many const_casts needed,
bool wex::frame::update_statusbar(stc* stc, const std::string& pane)
{
  if (stc == nullptr || m_is_closing)
  {
    return false;
  }
  
  std::stringstream text;

  if (pane == "PaneInfo")
  {
    if (stc->GetCurrentPos() == 0)
    {
      text << stc->GetLineCount();
    }
    else
    {
      int start;
      int end;
      const auto line = stc->GetCurrentLine() + 1;
      const auto pos = stc->GetCurrentPos() + 1 - stc->PositionFromLine(line - 1);
      stc->GetSelection(&start, &end);

      if (const int len  = end - start; len == 0) 
      {
        text << line << "," << pos;
      }
      else
      {
        if (stc->SelectionIsRectangle())
        {
          text << line << "," << pos << "," << stc->GetSelectedText().length();
        }
        else
        {
          if (const auto number_of_lines = 
            get_number_of_lines(stc->get_selected_text());
              number_of_lines <= 1) 
            text << line << "," << pos << "," << len;
          else
            text << line << "," << number_of_lines << "," << len;
        }
      }
    }
  }
  else if (pane == "PaneLexer")
  {
    if (!lexers::get()->theme().empty())
    {
      text << stc->get_lexer().display_lexer();
    }
  }
  else if (pane == "PaneFileType")
  {
    switch (stc->GetEOLMode())
    {
    case wxSTC_EOL_CRLF: text << "DOS"; break;
    case wxSTC_EOL_CR: text << "MAC"; break;
    case wxSTC_EOL_LF: text << "UNIX"; break;
    default: text << "UNKNOWN";
    }
  }
  else if (pane == "PaneMode")
  {
    text << stc->get_vi().mode().str();
  }
  else
  {
    return false;
  }

  return statustext(text.str(), pane);
}
