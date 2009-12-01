/******************************************************************************\
* File:          frame.cpp
* Purpose:       Implementation of wxExFrame class and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

//#include <wx/persist.h> 
#include <wx/config.h>
#include <wx/tooltip.h> // for GetTip
#include <wx/extension/frame.h>
#include <wx/extension/art.h>
#include <wx/extension/frd.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/printing.h>
#include <wx/extension/stc.h>
#include <wx/extension/tool.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

#if wxUSE_STATUSBAR
wxExStatusBar* wxExFrame::m_StatusBar = NULL;
std::map<wxString, wxExPane> wxExFrame::m_Panes;
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
  EVT_CLOSE(wxExFrame::OnClose)
  EVT_FIND(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_CLOSE(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_REPLACE(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_FIND_REPLACE_ALL(wxID_ANY, wxExFrame::OnFindDialog)
  EVT_MENU(wxID_FIND, wxExFrame::OnCommand)
  EVT_MENU(wxID_REPLACE, wxExFrame::OnCommand)
  EVT_MENU(ID_EDIT_FIND_NEXT, wxExFrame::OnCommand)
  EVT_MENU(ID_EDIT_FIND_PREVIOUS, wxExFrame::OnCommand)
#if wxUSE_STATUSBAR
  EVT_UPDATE_UI(ID_EDIT_STATUS_BAR, wxExFrame::OnUpdateUI)
#endif
END_EVENT_TABLE()

wxExFrame::wxExFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : wxFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, style, name)
  , m_FindReplaceDialog(NULL)
  , m_KeepPosAndSize(true)
{
  Initialize();

  // The wxPersistenceManager might be usefull in the future.
  //SetName("wxExFrame");
  //wxPersistenceManager::Get().RegisterAndRestore(this, this);

  if (wxConfigBase::Get()->ReadBool("Frame/Maximized", false))
  {
    Maximize(true);
  }

  SetSize(
    wxConfigBase::Get()->ReadLong("Frame/X", 100),
    wxConfigBase::Get()->ReadLong("Frame/Y", 100),
    wxConfigBase::Get()->ReadLong("Frame/Width", 450),
    wxConfigBase::Get()->ReadLong("Frame/Height", 350));
}

wxExFrame::wxExFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxFrame(parent, id, title, pos, size, style, name)
  , m_FindReplaceDialog(NULL)
  , m_KeepPosAndSize(false)
{
  Initialize();
}

wxExFrame::~wxExFrame()
{
  if (m_FindReplaceDialog != NULL)
  {
    m_FindReplaceDialog->Destroy();
  }

#if wxUSE_STATUSBAR
  delete m_StatusBar;
#endif
}

wxExGrid* wxExFrame::GetFocusedGrid()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, wxExGrid);
}

wxExListView* wxExFrame::GetFocusedListView()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, wxExListView);
}

wxExSTC* wxExFrame::GetFocusedSTC()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, wxExSTC);
}

#if wxUSE_STATUSBAR
const wxExPane wxExFrame::GetPane(int pane) const
{
  for (
    std::map<wxString, wxExPane>::const_iterator it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.m_No == pane)
    {
      return it->second;
    }
  }

  return wxExPane();
}

// This is a static method, so no const possible.
int wxExFrame::GetPaneField(const wxString& pane)
{
  std::map<wxString, wxExPane>::const_iterator it = m_Panes.find(pane);

  if (it != m_Panes.end())
  {
    return it->second.m_No;
  }

  return -1;
}
#endif // wxUSE_STATUSBAR

void wxExFrame::GetSearchText()
{
  wxExSTC* stc = GetSTC();

  if (stc != NULL && stc->IsShown())
  {
    stc->GetSearchText();
  }
  else
  {
    wxExGrid* grid = GetGrid();

    if (grid != NULL && grid->IsShown() )
    {
      grid->GetSearchText();
    }
  }
}

void wxExFrame::Initialize()
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new FileDropTarget(this));
#endif

  wxAcceleratorEntry entries[4];
  entries[0].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[1].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
  entries[2].Set(wxACCEL_NORMAL, WXK_F5, wxID_FIND);
  entries[3].Set(wxACCEL_NORMAL, WXK_F6, wxID_REPLACE);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
}

void wxExFrame::OnClose(wxCloseEvent& event)
{
  if (m_KeepPosAndSize)
  {
    // Set config values that might have changed.
    if (IsMaximized())
    {
      wxConfigBase::Get()->Write("Frame/Maximized", true);
    }
    else
    {
      wxConfigBase::Get()->Write("Frame/Maximized", false);
      const wxRect rect = GetRect();
      wxConfigBase::Get()->Write("Frame/X", (long)rect.GetX());
      wxConfigBase::Get()->Write("Frame/Y", (long)rect.GetY());
      wxConfigBase::Get()->Write("Frame/Width", (long)rect.GetWidth());
      wxConfigBase::Get()->Write("Frame/Height", (long)rect.GetHeight());
    }
  }

  event.Skip();
}

void wxExFrame::OnCommand(wxCommandEvent& command)
{
  wxExGrid* grid = GetGrid();
  wxExSTC* stc = GetSTC();
  wxExListView* lv = GetListView();
  wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  switch (command.GetId())
  {
  case wxID_FIND: 
    GetSearchText();
    
    if (m_FindReplaceDialog != NULL)
    {
      if (m_FindReplaceDialog->GetWindowStyle() & wxFR_REPLACEDIALOG)
      {
        m_FindReplaceDialog->Destroy();
        m_FindReplaceDialog = NULL;
      }
    }
    
    if (m_FindReplaceDialog == NULL)
    {
      m_FindReplaceDialog = new wxFindReplaceDialog(this, frd, _("Find")); 
    }
    
    m_FindReplaceDialog->Show();
    break;
    
  case wxID_REPLACE: 
    GetSearchText();
    
    if (m_FindReplaceDialog != NULL)
    {
      if (!(m_FindReplaceDialog->GetWindowStyle() & wxFR_REPLACEDIALOG))
      {
        m_FindReplaceDialog->Destroy();
        m_FindReplaceDialog = NULL;
      }
    }
    
    if (m_FindReplaceDialog == NULL)
    {
      m_FindReplaceDialog = new wxFindReplaceDialog(
        this, 
        frd,
        _("Replace"), 
        wxFR_REPLACEDIALOG); 
    }
      
    m_FindReplaceDialog->Show();
    break;
    
  case ID_EDIT_FIND_NEXT: 
    GetSearchText();

    if (stc != NULL)
    {
      stc->FindNext(); 
    }
    if (lv != NULL)
    {
      lv->FindNext(frd->GetFindString()); 
    }
    if (grid != NULL)
    {
      grid->FindNext(frd->GetFindString()); 
    }
    break;

  case ID_EDIT_FIND_PREVIOUS: 
    GetSearchText();

    if (stc != NULL)
    {
      stc->FindNext(false);
    }
    if (lv != NULL)
    {
      lv->FindNext(frd->GetFindString(), false);
    }
    if (grid != NULL)
    {
      grid->FindNext(frd->GetFindString(), false); 
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

#if wxUSE_TOOLBAR
wxToolBar* wxExFrame::OnCreateToolBar(
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_ToolBar = new wxExToolBar(this,
    id,
    wxDefaultPosition,
    wxDefaultSize,
    style,
    name);

  return m_ToolBar;
}
#endif

void wxExFrame::OnFindDialog(wxFindDialogEvent& event)
{
  wxASSERT(m_FindReplaceDialog != NULL);

  if (event.GetEventType() == wxEVT_COMMAND_FIND_CLOSE)
  {
    // Hiding instead of destroying, does not 
    // show the dialog next time.
    m_FindReplaceDialog->Destroy();
    m_FindReplaceDialog = NULL;
    return;
  }

  wxExFindReplaceData* frd = wxExFindReplaceData::Get();
  const bool find_next = (frd->GetFlags() & wxFR_DOWN);

  wxExSTC* stc = GetSTC();

  if (stc != NULL && stc->IsShown())
  {
    // Match word and regular expression do not work together.
    if (frd->MatchWord())
    {
      frd->SetIsRegularExpression(false);
    }

    if (
      event.GetEventType() == wxEVT_COMMAND_FIND ||
      event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
    {
      stc->FindNext(find_next);
    }
    else if (event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE)
    {
      stc->ReplaceNext(find_next);
    }
    else if (event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL)
    {
      stc->ReplaceAll(
        frd->GetFindString(), 
        frd->GetReplaceString());
    }
    else
    {
      wxFAIL;
    }

    return;
  }

  wxExListView* lv = GetListView();

  if (lv != NULL && lv->IsShown())
  {
    if (
      event.GetEventType() == wxEVT_COMMAND_FIND ||
      event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
    {
      lv->FindNext(frd->GetFindString(), find_next);
    }
    else
    {
      wxFAIL;
    }

    return;
  }

  wxExGrid* grid = GetGrid();

  if (grid != NULL && grid->IsShown())
  {
    if (
      event.GetEventType() == wxEVT_COMMAND_FIND ||
      event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
    {
      grid->FindNext(frd->GetFindString(), find_next);
    }
    else
    {
      wxFAIL;
    }

    return;
  }
}

void wxExFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  wxExSTC* stc = GetFocusedSTC();
  if (stc == NULL) return;

  switch (event.GetId())
  {
#if wxUSE_STATUSBAR
  case ID_EDIT_STATUS_BAR: stc->UpdateStatusBar("PaneLines"); break;
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
  wxExSTC* stc = GetFocusedSTC();

  if (stc != NULL)
  {
    return stc->Open(filename, line_number, match, flags);
  }

  return false;
}

#if wxUSE_STATUSBAR
void wxExFrame::SetupStatusBar(
  const std::vector<wxExPane>& panes,
  long style,
  wxWindowID id,
  const wxString& name)
{
  wxFrame::CreateStatusBar(panes.size(), style, id, name);

  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    std::vector<wxExPane>::const_iterator it = panes.begin();
    it != panes.end();
    ++it)
  {
    m_Panes[it->m_Name] = *it;
    styles[it->m_No] = it->GetStyle();
    widths[it->m_No] = it->GetWidth();
  }

  m_StatusBar->SetStatusStyles(panes.size(), styles);
  m_StatusBar->SetStatusWidths(panes.size(), widths);

  delete[] styles;
  delete[] widths;
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void wxExFrame::StatusBarDoubleClicked(int field, const wxPoint& point)
{
  if (field == GetPaneField("PaneLines"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->GotoDialog();
  }
  else if (field == GetPaneField("PaneLexer"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->LexerDialog();
  }
  else if (field == GetPaneField("PaneFileType"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->FileTypeMenu();
  }
  else if (field == GetPaneField("PaneItems"))
  {
    wxExListView* list = GetListView();
    if (list != NULL) list->GotoDialog();
  }
  else
  {
    // Clicking on another field, do nothing. 
  }
}

// This is a static method, so you cannot call wxFrame::SetStatusText.
void wxExFrame::StatusText(const wxString& text, const wxString& pane)
{
  if (m_StatusBar == NULL || m_Panes.empty())
  {
    // You did not ask for a status bar, so ignore all.
    return;
  }

  const int field = GetPaneField(pane);

  if (field >= 0)
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    m_StatusBar->SetStatusText(text, field);
  }
}

void wxExFrame::StatusText(const wxExFileName& filename, long flags)
{
  wxString text; // clear status bar for empty or not existing or not initialized file names

  if (filename.IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? filename.GetFullPath(): filename.GetFullName());

    text += path;

    if (filename.GetStat().IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): filename.GetStat().GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  StatusText(text);
}

#endif // wxUSE_STATUSBAR

wxExManagedFrame::wxExManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : wxExFrame(parent, id, title, style, name)
{
  m_Manager.SetManagedWindow(this);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxExPrinting::Get()->GetHtmlPrinter()->SetParentWindow(this);
#endif
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}

int wxExPane::m_Total = 0;

#if wxUSE_STATUSBAR
BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(wxExStatusBar::OnMouse)
  EVT_LEFT_DCLICK(wxExStatusBar::OnMouse)
  EVT_MOTION(wxExStatusBar::OnMouse)
END_EVENT_TABLE()

wxExStatusBar::wxExStatusBar(
  wxExFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
{
  bool found = false;

  for (int i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        if (event.ButtonDClick())
        {
          m_Frame->StatusBarDoubleClicked(i, event.GetPosition());
        }
        else if (event.ButtonDown())
        {
          m_Frame->StatusBarClicked(i, event.GetPosition());
        }
        // Show tooltip if tooltip is available, and not yet tooltip presented.
        else if (event.Moving())
        {
          if (!m_Frame->m_Panes.empty())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != m_Frame->GetPane(i).m_Helptext)
            {
              SetToolTip(m_Frame->GetPane(i).m_Helptext);
            }
          }
        }
        else
        {
          wxFAIL;
        }

        found = true;
      }
    }
  }

  event.Skip();
}
#endif //wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
wxExToolBar::wxExToolBar(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxToolBar(parent, id, pos, size, style, name)
{
}

wxToolBarToolBase* wxExToolBar::AddTool(int toolId)
{
  const wxExStockArt art(toolId);

  return wxToolBar::AddTool(
    toolId, 
    wxEmptyString,
    art.GetBitmap(wxART_TOOLBAR, GetToolBitmapSize()),
    wxGetStockLabel(toolId, wxSTOCK_NOFLAGS));
}
#endif // wxUSE_TOOLBAR

/// Offers a find combobox that allows you to find text
/// on a current STC on an wxExFrameWithHistory.
class ComboBox : public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values from FindReplace from config.
  ComboBox(
    wxWindow* parent,
    wxExFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ComboBox, wxComboBox)
  EVT_CHAR(ComboBox::OnKey)
  EVT_MENU(wxID_DELETE, ComboBox::OnCommand)
END_EVENT_TABLE()

ComboBox::ComboBox(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxComboBox(parent, id, wxEmptyString, pos, size)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);

  SetFont(wxConfigBase::Get()->ReadObject("FindFont", 
    wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

  wxExComboBoxFromString(
    this, 
    wxConfigBase::Get()->Read(wxExFindReplaceData::Get()->GetTextFindWhat()));

  // And override the value set by previous, as we want text to be same as in Find.
  SetValue(wxExFindReplaceData::Get()->GetFindString());
}

void ComboBox::OnCommand(wxCommandEvent& event)
{
  // README: The delete key default behaviour does not delete the char right from insertion point.
  // Instead, the event is sent to the editor and a char is deleted from the editor.
  // Therefore implement the delete here.
  switch (event.GetId())
  {
  case wxID_DELETE:
    Remove(GetInsertionPoint(), GetInsertionPoint() + 1);
    break;
  default:
    wxFAIL;
    break;
  }
}

void ComboBox::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    wxExSTC* stc = m_Frame->GetSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());
      wxExFindReplaceData::Get()->SetFindString(GetValue());

      // And keep the changed text in the combo box.
      const wxString text = wxExComboBoxToString(this);

      wxConfigBase::Get()->Write(
        wxExFindReplaceData::Get()->GetTextFindWhat(), 
        text);

      Clear(); // so wxExComboBoxFromString can append again
      wxExComboBoxFromString(this, text);

      SetValue(wxExFindReplaceData::Get()->GetFindString());
    }
  }
  else
  {
    event.Skip();
  }
}

enum
{
  ID_MATCH_WHOLE_WORD = 100,
  ID_MATCH_CASE,
  ID_REGULAR_EXPRESSION,
};

BEGIN_EVENT_TABLE(wxExFindToolBar, wxAuiToolBar)
  EVT_CHECKBOX(ID_MATCH_WHOLE_WORD, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_MATCH_CASE, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_REGULAR_EXPRESSION, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_DOWN, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_UP, wxExFindToolBar::OnCommand)
  EVT_UPDATE_UI(wxID_DOWN, wxExFindToolBar::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UP, wxExFindToolBar::OnUpdateUI)
END_EVENT_TABLE()

wxExFindToolBar::wxExFindToolBar(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id)
  : wxAuiToolBar(parent, id)
  , m_Frame(frame)
{
  Initialize();

  // And place the controls on the toolbar.
  AddControl(m_ComboBox);
  AddSeparator();

  AddTool(
    wxID_DOWN, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find next"));
  AddTool(
    wxID_UP, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find previous"));
  AddSeparator();

  AddControl(m_MatchWholeWord);
  AddControl(m_MatchCase);
  AddControl(m_RegularExpression);

  Realize();
}

void wxExFindToolBar::Initialize()
{
#ifdef __WXMSW__
  const wxSize size(150, 20);
#else
  const wxSize size(150, -1);
#endif
  m_ComboBox = new ComboBox(this, m_Frame, wxID_ANY, wxDefaultPosition, size);

  m_MatchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, wxExFindReplaceData::Get()->GetTextMatchCase());

  m_MatchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, wxExFindReplaceData::Get()->GetTextMatchWholeWord());

  m_RegularExpression= new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, wxExFindReplaceData::Get()->GetTextRegEx());

  m_MatchCase->SetValue(wxExFindReplaceData::Get()->MatchCase());
  m_MatchWholeWord->SetValue(wxExFindReplaceData::Get()->MatchWord());
  m_RegularExpression->SetValue(wxExFindReplaceData::Get()->IsRegularExpression());
}

void wxExFindToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_DOWN:
  case wxID_UP:
    {
      wxExSTC* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        stc->FindNext(m_ComboBox->GetValue(), (event.GetId() == wxID_DOWN));
      }
    }
    break;

  case ID_MATCH_WHOLE_WORD:
    wxExFindReplaceData::Get()->SetMatchWord(
      m_MatchWholeWord->GetValue());
    break;
  case ID_MATCH_CASE:
    wxExFindReplaceData::Get()->SetMatchCase(
      m_MatchCase->GetValue());
    break;
  case ID_REGULAR_EXPRESSION:
    wxExFindReplaceData::Get()->SetIsRegularExpression(
      m_RegularExpression->GetValue());
    break;

  default:
    wxFAIL;
    break;
  }
}

void wxExFindToolBar::OnUpdateUI(wxUpdateUIEvent& event)
{
  event.Enable(!m_ComboBox->GetValue().empty());
}

#endif // wxUSE_GUI
