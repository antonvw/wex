////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wxExFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/persist/toplevel.h>
#include <wx/tokenzr.h> 
#include <wx/extension/frame.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/grid.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listview.h>
#include <wx/extension/printing.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

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
      wxExOpenFiles(m_Frame, filenames);
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

#if wxUSE_STATUSBAR
  delete m_StatusBar;
#endif
}

wxExGrid* wxExFrame::GetGrid()
{
  // If we had a find focus on grid component, return that one.
  if (m_FindFocus != NULL && m_FindFocus->IsShown())
  {
    wxExGrid* win = dynamic_cast<wxExGrid*>(m_FindFocus);
    
    if (win != NULL)
    {
      return win;
    }
  }
  
  // If current focus is on grid component, return that one.  
  wxWindow* win = wxWindow::FindFocus();
  wxExGrid* grid = dynamic_cast<wxExGrid*>(win);
  return grid;
}

wxExListView* wxExFrame::GetListView()
{
  // If we had a find focus on listview component, return that one.
  if (m_FindFocus != NULL && m_FindFocus->IsShown())
  {
    wxExListView* lv = dynamic_cast<wxExListView*>(m_FindFocus);
    
    if (lv != NULL)
    {
      return lv;
    }
  }
  
  // If current focus is on listview component, return that one.  
  wxWindow* win = wxWindow::FindFocus();
  wxExListView* lv = dynamic_cast<wxExListView*>(win);
  return lv;
}

wxExSTC* wxExFrame::GetSTC()
{
  // If we had a find focus on stc component, return that one.
  if (m_FindFocus != NULL && m_FindFocus->IsShown())
  {
    wxExSTC* stc = dynamic_cast<wxExSTC*>(m_FindFocus);
    
    if (stc != NULL)
    {
      return stc;
    }
  }

  // If current focus is on stc component, return that one.  
  wxWindow* win = wxWindow::FindFocus();
  wxExSTC* stc = dynamic_cast<wxExSTC*>(win);
  return stc;
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
  case wxID_FIND: 
    {
    if (m_FindReplaceDialog != NULL)
    {
      m_FindReplaceDialog->Destroy();
    }
    
    m_FindFocus = wxWindow::FindFocus();

    // If stc text is selected, copy to find replace data.
    wxExSTC* stc = GetSTC();
    
    if (stc != NULL)
    {
      stc->GetFindString();
    }
    
    m_FindReplaceDialog = new wxFindReplaceDialog(
      this, wxExFindReplaceData::Get(), _("Find")); 
    m_FindReplaceDialog->Show();
    }
    break;
    
  case wxID_REPLACE: 
    {
    if (m_FindReplaceDialog != NULL)
    {
      m_FindReplaceDialog->Destroy();
    }
    
    m_FindFocus = wxWindow::FindFocus();
    
    // If stc text is selected, copy to find replace data.
    wxExSTC* stc = GetSTC();
    
    if (stc != NULL)
    {
      stc->GetFindString();
    }
    
    m_FindReplaceDialog = new wxFindReplaceDialog(
      this, 
      wxExFindReplaceData::Get(),
      _("Replace"), 
      wxFR_REPLACEDIALOG); 
    m_FindReplaceDialog->Show();
    }
    break;
    
  case wxID_OPEN:
    if (!command.GetString().empty())
    {
      wxArrayString files;
      wxStringTokenizer tkz(command.GetString());
      wxExSTC* stc = GetSTC();

      if ( stc != NULL && 
          (command.GetString().Contains("*") || command.GetString().Contains("?")))
      {
        wxSetWorkingDirectory(stc->GetFileName().GetPath());
      }
      
      while (tkz.HasMoreTokens())
      {
        const wxString token = tkz.GetNextToken();

        if (token.Contains("*") || token.Contains("?"))
        {
          files.Add(token);
        }
        else
        {
          wxFileName file(token);

          if (file.IsRelative() && stc != NULL)
          {
            file.MakeAbsolute(stc->GetFileName().GetPath());

            if (!file.FileExists())
            {
              wxLogError(_("Cannot locate file") + ": " + token);
            }
            else
            {
              files.Add(file.GetFullPath());
            }
          }
          else
          {
            files.Add(file.GetFullPath());
          }
        }
      }

      wxExOpenFiles(this, files);
    }
    else
    {
      wxExOpenFilesDialog(this);
    }
    break;
      
  case ID_VIEW_MENUBAR:
    if (GetMenuBar() != NULL)
    {
      SetMenuBar(NULL);
    }
    else
    {
      SetMenuBar(m_MenuBar);
    }
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
    if (!(GetWindowStyleFlag() & wxCAPTION))
    {
      SetWindowStyleFlag(wxDEFAULT_FRAME_STYLE);
      Refresh();
    }
    else
    {
      SetWindowStyleFlag(GetWindowStyleFlag() & ~wxCAPTION);
      Refresh();
    }
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
    if (stc != NULL) 
    {
      UpdateStatusBar(stc, "PaneInfo"); 
    }
    }
    break;
#endif

  case ID_VIEW_MENUBAR:
    if (GetMenuBar() != NULL)
    {
      event.Check(GetMenuBar()->IsShown());
    }
    else
    {
      event.Check(false);
    }
    break;

#if wxUSE_STATUSBAR
  case ID_VIEW_STATUSBAR:
    if (GetStatusBar() != NULL)
    {
      event.Check(GetStatusBar()->IsShown());
    }
    else
    {
      event.Check(false);
    }
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
  long flags)
{
  wxExSTC* stc = GetSTC();

  if (stc != NULL)
  {
    return stc->Open(filename, line_number, match, flags);
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

    wxExVCSCommandOnSTC(
      vcs.GetCommand(), filename.GetLexer(), stc);
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

  if (stc != NULL)
  {
    stc->SetText(text);
  }
  else
  {
    wxLogMessage(text);
  }

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
  
  if (
    !m_IsCommand &&
    !wxConfigBase::Get()->ReadBool("ShowMenuBar", true))
  {
    wxFrame::SetMenuBar(NULL);
  }
  else
  {
    wxFrame::SetMenuBar(bar);
  }
}

#if wxUSE_STATUSBAR
void wxExFrame::SetupStatusBar(
  const std::vector<wxExStatusBarPane>& panes,
  long style,
  const wxString& name)
{
  wxFrame::CreateStatusBar(panes.size(), style, ID_UPDATE_STATUS_BAR, name);
  m_StatusBar->SetFields(panes);
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void wxExFrame::StatusBarDoubleClicked(const wxString& pane)
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
      wxExListView* list = GetListView();
      
      if (list != NULL) list->GotoDialog();
    }
  }
  else if (pane == "PaneLexer")
  {
    wxExSTC* stc = GetSTC();

    if (stc != NULL && wxExLexers::Get()->GetCount() > 0)
    {
      wxString lexer = stc->GetLexer().GetDisplayLexer();

      if (wxExLexers::Get()->ShowDialog(this, lexer))
      {
        stc->SetLexer(
          lexer,
          true); // fold: refresh
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
void wxExFrame::StatusText(const wxString& text, const wxString& pane)
{
  if (m_StatusBar != NULL)
  {
    m_StatusBar->SetStatusText(text, pane);
  }
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void wxExFrame::UpdateStatusBar(const wxListView* lv)
{
  if (lv->IsShown())
  {
    const wxString text = (lv->GetSelectedItemCount() == 0 ?
      wxString::Format("%d", lv->GetItemCount()):
      wxString::Format("%d,%d", lv->GetItemCount(), lv->GetSelectedItemCount()));
      
    StatusText(text, "PaneInfo");
  }
}

// Do not make it const, too many const_casts needed,
// I thought that might cause crash in rect selection, but it didn't.
void wxExFrame::UpdateStatusBar(wxExSTC* stc, const wxString& pane)
{
  wxString text;

  if (pane == "PaneInfo")
  {
    if (stc->GetCurrentPos() == 0) text = wxString::Format("%d", stc->GetLineCount());
    else
    {
      int start;
      int end;
      stc->GetSelection(&start, &end);

      const int len  = end - start;
      const int line = stc->GetCurrentLine() + 1;
      const int pos = stc->GetCurrentPos() + 1 - stc->PositionFromLine(line - 1);

      if (len == 0) text = wxString::Format("%d,%d", line, pos);
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
    wxFAIL;
  }

  StatusText(text, pane);
}
#endif

#endif // wxUSE_GUI
