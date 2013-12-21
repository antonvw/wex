////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wxExFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wx/persist/toplevel.h>
#include <wx/tokenzr.h> 
#include <wx/extension/frame.h>
#include <wx/extension/defs.h>
#include <wx/extension/filename.h>
#include <wx/extension/frd.h>
#include <wx/extension/grid.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listview.h>
#include <wx/extension/printing.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vcsentry.h>

#if wxUSE_GUI

#define wxCAST_TO(classname)                                 \
  if (m_FindFocus != NULL && m_FindFocus->IsShown())         \
  {                                                          \
    classname* win = dynamic_cast<classname*>(m_FindFocus);  \
                                                             \
    if (win != NULL)                                         \
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
  if (m_FindReplaceDialog != NULL)                           \
  {                                                          \
    m_FindReplaceDialog->Destroy();                          \
  }                                                          \
                                                             \
  m_FindFocus = wxWindow::FindFocus();                       \
                                                             \
  wxExSTC* stc = GetSTC();                                   \
                                                             \
  if (stc != NULL)                                           \
  {                                                          \
    stc->GetFindString();                                    \
  }                                                          \
                                                             \
  m_FindReplaceDialog = new wxFindReplaceDialog(             \
    this, wxExFindReplaceData::Get(), text, dlg);            \
  m_FindReplaceDialog->Show();                               \
};                                                           \
  
const int ID_UPDATE_STATUS_BAR = 900;

#if wxUSE_STATUSBAR
wxExStatusBar* wxExFrame::m_StatusBar = NULL;
#endif

#if wxUSE_DRAG_AND_DROP
class FileDropTarget : public wxFileDropTarget
{
public:
  FileDropTarget(wxExFrame* frame) 
    : m_Frame(frame){;};
private:
  virtual bool OnDropFiles(
    wxCoord x, 
    wxCoord y, 
    const wxArrayString& filenames) {
      wxExOpenFiles(m_Frame, wxExToVectorString(filenames).Get());
      return true;}

  wxExFrame* m_Frame;
};
#endif

BEGIN_EVENT_TABLE(wxExFrame, wxFrame)
  EVT_FIND(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_CLOSE(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_REPLACE(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_REPLACE_ALL(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_MENU(wxID_FIND, wxExFrame::OnCommand)
  EVT_MENU(wxID_REPLACE, wxExFrame::OnCommand)
  EVT_MENU(wxID_OPEN, wxExFrame::OnCommand)
  EVT_MENU(ID_VIEW_MENUBAR, wxExFrame::OnCommand)
  EVT_MENU(ID_VIEW_STATUSBAR, wxExFrame::OnCommand)
  EVT_MENU(ID_VIEW_TITLEBAR, wxExFrame::OnCommand)
  EVT_UPDATE_UI(ID_VIEW_MENUBAR, wxExFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATUSBAR, wxExFrame::OnUpdateUI)
#if wxUSE_STATUSBAR
  EVT_UPDATE_UI(ID_UPDATE_STATUS_BAR, wxExFrame::OnUpdateUI)
#endif
END_EVENT_TABLE()

wxExFrame::wxExFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style)
  : wxFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, style, "wxExFrame")
  , m_FindReplaceDialog(NULL)
  , m_MenuBar(NULL)
  , m_FindFocus(NULL)
  , m_IsCommand(false)
{
  Initialize();

  wxPersistentRegisterAndRestore(this);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxExPrinting::Get()->GetHtmlPrinter()->SetParentWindow(this);
#endif
}

wxExFrame::~wxExFrame()
{
  if (m_FindReplaceDialog != NULL)
  {
    m_FindReplaceDialog->Destroy();
  }
  
  wxConfigBase::Get()->Write("ShowMenuBar", 
    GetMenuBar() != NULL && GetMenuBar()->IsShown());
}

wxExGrid* wxExFrame::GetGrid()
{
  wxCAST_TO(wxExGrid);
}

wxExListView* wxExFrame::GetListView()
{
  wxCAST_TO(wxExListView);
}

wxString wxExFrame::GetStatusText(const wxString& pane)
{
  return (m_StatusBar == NULL ? wxString(wxEmptyString): m_StatusBar->GetStatusText(pane));
}

wxExSTC* wxExFrame::GetSTC()
{
  wxCAST_TO(wxExSTC);
}
  
void wxExFrame::Initialize()
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new FileDropTarget(this));
#endif

  wxAcceleratorEntry entries[4];
  entries[0].Set(wxACCEL_NORMAL, WXK_F5, wxID_FIND);
  entries[1].Set(wxACCEL_NORMAL, WXK_F6, wxID_REPLACE);
  entries[2].Set(wxACCEL_CTRL, (int)'I', ID_VIEW_MENUBAR);
  entries[3].Set(wxACCEL_CTRL, (int)'T', ID_VIEW_TITLEBAR);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
}

void wxExFrame::OnCommand(wxCommandEvent& command)
{
  m_IsCommand = true;

  switch (command.GetId())
  {
  case wxID_FIND: FIND_REPLACE(_("Find"), 0 ); break;
  case wxID_REPLACE: FIND_REPLACE( _("Replace") , wxFR_REPLACEDIALOG ); break;
  case wxID_OPEN:
    if (!command.GetString().empty())
    {
      wxExSTC* stc = GetSTC();

      if (stc != NULL)
      {
        wxSetWorkingDirectory(stc->GetFileName().GetPath());
      }
      
      wxExOpenFiles(this, wxExToVectorString(command.GetString()).Get());
    }
    else
    {
      wxExOpenFilesDialog(this);
    }
    break;
      
  case ID_VIEW_MENUBAR:
    SetMenuBar(GetMenuBar() != NULL ? NULL: m_MenuBar);
    break;

  case ID_VIEW_STATUSBAR:
#if wxUSE_STATUSBAR
    if (GetStatusBar() != NULL)
    {
      GetStatusBar()->Show(!GetStatusBar()->IsShown());
      SendSizeEvent();
    }
#endif    
    break;
    
  case ID_VIEW_TITLEBAR:
    SetWindowStyleFlag(!(GetWindowStyleFlag() & wxCAPTION) ? 
      wxDEFAULT_FRAME_STYLE:
      GetWindowStyleFlag() & ~wxCAPTION);
    Refresh();
    break;

  default: wxFAIL; break;
  }
}

#if wxUSE_STATUSBAR
wxStatusBar* wxExFrame::OnCreateStatusBar(
  int number,
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_StatusBar = new wxExStatusBar(this, id, style, name);
  m_StatusBar->SetFieldsCount(number);
  return m_StatusBar;
}
#endif

void wxExFrame::OnFindDialog(wxFindDialogEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_FIND_CLOSE)
  {
    wxASSERT(m_FindReplaceDialog != NULL);

    // Hiding instead of destroying, does not 
    // show the dialog next time.
    m_FindReplaceDialog->Destroy();
    m_FindReplaceDialog = NULL;
  }
  else
  {
    if (m_FindFocus != NULL)
    {
      wxPostEvent(m_FindFocus, event);
    }
  }
}

void wxExFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
#if wxUSE_STATUSBAR
    case ID_UPDATE_STATUS_BAR:
    {
    wxExSTC* stc = GetSTC();
    
    if (stc != NULL && stc->HasFocus()) 
    {
      UpdateStatusBar(stc, "PaneInfo"); 
      UpdateStatusBar(stc, "PaneLexer"); 
      UpdateStatusBar(stc, "PaneFileType"); 
    }
    else
    {
      wxExListView* lv = GetListView();
      
      if (lv != NULL && lv->HasFocus())
      {
        UpdateStatusBar(lv);
      }
    }
    }
    break;
#endif

  case ID_VIEW_MENUBAR:
    (GetMenuBar() != NULL ? event.Check(GetMenuBar()->IsShown()): event.Check(false));
    break;

#if wxUSE_STATUSBAR
  case ID_VIEW_STATUSBAR:
    (GetStatusBar() != NULL ? event.Check(GetStatusBar()->IsShown()): event.Check(false));
    break;
#endif
      
  default:
    wxFAIL;
    break;
  }
}

bool wxExFrame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags)
{
  wxExSTC* stc = GetSTC();

  if (stc != NULL)
  {
    return stc->Open(filename, line_number, match, col_number, flags);
  }

  return false;
}

bool wxExFrame::OpenFile(
  const wxExFileName& filename,
  const wxExVCSEntry& vcs,
  long flags)
{
  wxExSTC* stc = GetSTC();

  if (stc != NULL)
  {
    stc->SetText(vcs.GetOutput());
    wxExVCSCommandOnSTC(vcs.GetCommand(), filename.GetLexer(), stc);
  }
  else
  {
    wxLogMessage(vcs.GetOutput());
  }

  return true;
}

bool wxExFrame::OpenFile(
  const wxString& filename,
  const wxString& text,
  long flags)
{
  wxExSTC* stc = GetSTC();

  if (stc != NULL) stc->SetText(text); else wxLogMessage(text);

  return true;
}
    
void wxExFrame::SetFindFocus(wxWindow* focus)
{
  m_FindFocus = focus;
}

void wxExFrame::SetMenuBar(wxMenuBar* bar)
{
  if (bar != NULL)
  {
    m_MenuBar = bar;
  }
  
  wxFrame::SetMenuBar(
   !m_IsCommand && !wxConfigBase::Get()->ReadBool("ShowMenuBar", true) ? NULL: bar);
}

#if wxUSE_STATUSBAR
wxExStatusBar* wxExFrame::SetupStatusBar(
  const std::vector<wxExStatusBarPane>& panes,
  long style,
  const wxString& name)
{
  wxFrame::CreateStatusBar(panes.size(), style, ID_UPDATE_STATUS_BAR, name);
  m_StatusBar->SetFields(panes);
  return m_StatusBar;
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void wxExFrame::StatusBarClicked(const wxString& pane)
{
  if (pane == "PaneInfo")
  {
    wxExSTC* stc = GetSTC();
    
    if (stc != NULL) 
    {
      stc->GotoDialog();
    }
    else
    {
      wxExListView* lv = GetListView();
      
      if (lv != NULL) lv->GotoDialog();
    }
  }
  else if (pane == "PaneLexer")
  {
    wxExSTC* stc = GetSTC();

    if (stc != NULL)
    {
      wxString lexer = stc->GetLexer().GetDisplayLexer();

      if (wxExLexers::Get()->ShowDialog(stc, lexer))
      {
        if (lexer.empty())
        {
          stc->ResetLexer();
        }
        else
        {
          stc->SetLexer(lexer, true); // allow fold
        }
      }
    }
  }
  else if (pane == "PaneFileType")
  {
    wxExSTC* stc = GetSTC();
    
    if (stc != NULL) stc->FileTypeMenu();
  }
  else
  {
    // Clicking on another field, do nothing. 
  }
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
bool wxExFrame::StatusText(const wxString& text, const wxString& pane)
{
  return (m_StatusBar == NULL ? false: m_StatusBar->SetStatusText(text, pane));
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
bool wxExFrame::UpdateStatusBar(const wxListView* lv)
{
  if (lv->IsShown())
  {
    const wxString text = (lv->GetSelectedItemCount() == 0 ?
      wxString::Format("%d", lv->GetItemCount()):
      wxString::Format("%d,%d", lv->GetItemCount(), lv->GetSelectedItemCount()));
      
    return StatusText(text, "PaneInfo");
  }
  
  return false;
}

// Do not make it const, too many const_casts needed,
bool wxExFrame::UpdateStatusBar(wxExSTC* stc, const wxString& pane)
{
  if (stc == NULL)
  {
    return false;
  }
  
  wxString text;

  if (pane == "PaneInfo")
  {
    if (stc->GetCurrentPos() == 0)
    {
      text = wxString::Format("%d", stc->GetLineCount());
    }
    else
    {
      int start;
      int end;
      stc->GetSelection(&start, &end);

      const int len  = end - start;
      const int line = stc->GetCurrentLine() + 1;
      const int pos = stc->GetCurrentPos() + 1 - stc->PositionFromLine(line - 1);

      if (len == 0) 
      {
        text = wxString::Format("%d,%d", line, pos);
      }
      else
      {
        if (stc->SelectionIsRectangle())
        {
          text = wxString::Format("%d,%d,%d", line, pos, stc->GetSelectedText().length());
        }
        else
        {
          // There might be NULL's inside selection.
          // So use the GetSelectedTextRaw variant.
          const int number_of_lines = 
            wxExGetNumberOfLines(stc->GetSelectedTextRaw());
            
          if (number_of_lines <= 1) 
            text = wxString::Format("%d,%d,%d", line, pos, len);
          else
            text = wxString::Format("%d,%d,%d", line, number_of_lines, len);
        }
      }
    }
  }
  else if (pane == "PaneLexer")
  {
    text = stc->GetLexer().GetDisplayLexer();
  }
  else if (pane == "PaneFileType")
  {
    switch (stc->GetEOLMode())
    {
    case wxSTC_EOL_CRLF: text = "DOS"; break;
    case wxSTC_EOL_CR: text = "MAC"; break;
    case wxSTC_EOL_LF: text = "UNIX"; break;
    default: text = "UNKNOWN";
    }
  }
  else
  {
    return false;
  }

  return StatusText(text, pane);
}
#endif

#endif // wxUSE_GUI
