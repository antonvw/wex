////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wxExToolBar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/art.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

// Cannot use wxNewId here, as these are used in a switch statement.
enum
{
  ID_MATCH_WHOLE_WORD = 100,
  ID_MATCH_CASE,
  ID_REGULAR_EXPRESSION,
  ID_HEX_MODE,
  ID_SYNC_MODE 
};

// Support class.
// Offers a find text ctrl that allows you to find text
// on a current STC on an wxExFrame.
class wxExTextCtrl : public wxTextCtrl
{
public:
  /// Constructor. Fills the text ctrl with value 
  /// from FindReplace from config.
  wxExTextCtrl(
    wxWindow* parent,
    wxExFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  /// Finds current value in control.
  void Find(bool find_next = true, bool restore_position = false);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);
  void OnFocus(wxFocusEvent& event);
private:
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxExToolBar, wxAuiToolBar)
  EVT_CHECKBOX(ID_HEX_MODE, wxExToolBar::OnCommand)
  EVT_CHECKBOX(ID_SYNC_MODE, wxExToolBar::OnCommand)
END_EVENT_TABLE()

wxExToolBar::wxExToolBar(wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiToolBar(frame, id, pos, size, style | wxAUI_TB_HORZ_TEXT)
  , m_Frame(frame)
  , m_HexMode(NULL)
  , m_SyncMode(NULL)
{
}

void wxExToolBar::AddControls()
{
  AddTool(wxID_OPEN);
  AddTool(wxID_SAVE);
  AddTool(wxID_PRINT);
  AddTool(wxID_FIND);

  AddControl(
    m_HexMode = new wxCheckBox(
      this,
      ID_HEX_MODE,
      "Hex"));

  AddControl(
    m_SyncMode = new wxCheckBox(
      this,
      ID_SYNC_MODE,
      "Sync"));

#if wxUSE_TOOLTIPS
  m_HexMode->SetToolTip(_("Open in hex mode"));
  m_SyncMode->SetToolTip(_("Synchronize modified files"));
#endif

  m_HexMode->SetValue(wxConfigBase::Get()->ReadBool("HexMode", false));
  m_SyncMode->SetValue(wxConfigBase::Get()->ReadBool("AllowSync", true));

  Realize();
}

wxAuiToolBarItem* wxExToolBar::AddTool(
  int toolId,
  const wxString& label,
  const wxBitmap& bitmap,
  const wxString& shortHelp,
  wxItemKind kind)
{
  const wxExStockArt art(toolId);

  if (art.GetBitmap(wxART_TOOLBAR).IsOk())
  {
    return wxAuiToolBar::AddTool(
      toolId, 
      wxEmptyString,
      art.GetBitmap(wxART_TOOLBAR),
      wxGetStockLabel(toolId, wxSTOCK_NOFLAGS),
      kind);
  }
  else
  {
    return wxAuiToolBar::AddTool(
      toolId, 
      label,
      bitmap,
      shortHelp,
      kind);
  }
}

void wxExToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case ID_HEX_MODE:
    wxConfigBase::Get()->Write("HexMode", m_HexMode->GetValue());
    break;

  case ID_SYNC_MODE:
    wxConfigBase::Get()->Write("AllowSync", m_SyncMode->GetValue());
    break;

  default: 
    wxFAIL;
    break;
  }
}

BEGIN_EVENT_TABLE(wxExFindToolBar, wxExToolBar)
  EVT_CHECKBOX(ID_MATCH_WHOLE_WORD, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_MATCH_CASE, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_REGULAR_EXPRESSION, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_DOWN, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_UP, wxExFindToolBar::OnCommand)
  EVT_UPDATE_UI(wxID_DOWN, wxExFindToolBar::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UP, wxExFindToolBar::OnUpdateUI)
END_EVENT_TABLE()

wxExFindToolBar::wxExFindToolBar(
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExToolBar(frame, id, pos, size, style)
{
  Initialize();

  // And place the controls on the toolbar.
  AddControl(m_FindCtrl);

  AddTool(
    wxID_DOWN, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR),
    _("Find next"));
  AddTool(
    wxID_UP, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR),
    _("Find previous"));

  AddControl(m_MatchWholeWord);
  AddControl(m_MatchCase);
  AddControl(m_IsRegularExpression);

  Realize();
}

void wxExFindToolBar::Initialize()
{
  m_FindCtrl = new wxExTextCtrl(this, GetFrame());

  m_MatchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, wxExFindReplaceData::Get()->GetTextMatchCase());

  m_MatchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, wxExFindReplaceData::Get()->GetTextMatchWholeWord());

  m_IsRegularExpression = new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, wxExFindReplaceData::Get()->GetTextRegEx());

#if wxUSE_TOOLTIPS
  m_MatchCase->SetToolTip(_("Search case sensitive"));
  m_MatchWholeWord->SetToolTip(_("Search matching words"));
  m_IsRegularExpression->SetToolTip(_("Search using regular expressions"));
#endif

  m_MatchCase->SetValue(wxExFindReplaceData::Get()->MatchCase());
  m_MatchWholeWord->SetValue(wxExFindReplaceData::Get()->MatchWord());
  m_IsRegularExpression->SetValue(wxExFindReplaceData::Get()->UseRegularExpression());
}

void wxExFindToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_DOWN:
  case wxID_UP:
    m_FindCtrl->Find(event.GetId() == wxID_DOWN);
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
    wxExFindReplaceData::Get()->SetUseRegularExpression(
      m_IsRegularExpression->GetValue());
    break;
  default:
    wxFAIL;
    break;
  }
}

void wxExFindToolBar::OnUpdateUI(wxUpdateUIEvent& event)
{
  event.Enable(!m_FindCtrl->GetValue().empty());
}

// Implementation of support class.

BEGIN_EVENT_TABLE(wxExTextCtrl, wxTextCtrl)
  EVT_SET_FOCUS(wxExTextCtrl::OnFocus)
  EVT_TEXT(wxID_ANY, wxExTextCtrl::OnCommand)
  EVT_TEXT_ENTER(wxID_ANY, wxExTextCtrl::OnEnter)
END_EVENT_TABLE()

wxExTextCtrl::wxExTextCtrl(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, 
      id,
      wxExFindReplaceData::Get()->GetFindString(), 
      pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);
}

void wxExTextCtrl::Find(bool find_next, bool restore_position)
{
  // We cannot use events here, as OnFindDialog in stc uses frd data,
  // whereas we need the GetValue here.
  wxExSTC* stc = m_Frame->GetSTC();

  if (stc != NULL)
  {
    m_Frame->SetFindFocus(stc);
  
    if (restore_position)
    {
      stc->PositionRestore();
    }
    
    stc->FindNext(
      GetValue(), 
      wxExFindReplaceData::Get()->STCFlags(),
      find_next);
  }
}

void wxExTextCtrl::OnCommand(wxCommandEvent& event)
{
  event.Skip();
  Find(true, true);
}

void wxExTextCtrl::OnEnter(wxCommandEvent& event)
{
  if (!GetValue().empty())
  {
    wxExFindReplaceData::Get()->SetFindString(GetValue());
    Find();
  }
}

void wxExTextCtrl::OnFocus(wxFocusEvent& event)
{
  wxExSTC* stc = m_Frame->GetSTC();

  if (stc != NULL)
  {
    stc->PositionSave();
  }
  
  event.Skip();
}

#endif // wxUSE_GUI
