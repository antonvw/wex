////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wxExToolBar class
// Author:    Anton van Wezenbeek
// Created:   2010-03-26
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/art.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI
#if wxUSE_AUI

// Cannot use wxNewId here, as these are used in a switch statement.
enum
{
  ID_MATCH_WHOLE_WORD = 100,
  ID_MATCH_CASE,
  ID_REGULAR_EXPRESSION,
  ID_EDIT_HEX_MODE,
  ID_SYNC_MODE,
};

// Support class.
// Offers a find text ctrl that allows you to find text
// on a current STC on an wxExFrame.
class FindString : public wxTextCtrl
{
public:
  /// Constructor. Fills the text ctrl with value 
  /// from FindReplace from config.
  FindString(
    wxWindow* parent,
    wxExFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(wxExToolBar, wxAuiToolBar)
  EVT_CHECKBOX(ID_EDIT_HEX_MODE, wxExToolBar::OnCommand)
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
  AddSeparator();
  AddTool(wxID_FIND);
  AddSeparator();

  AddControl(
    m_HexMode = new wxCheckBox(
      this,
      ID_EDIT_HEX_MODE,
      "Hex"));

  AddControl(
    m_SyncMode = new wxCheckBox(
      this,
      ID_SYNC_MODE,
      "Sync"));

#if wxUSE_TOOLTIPS
  m_HexMode->SetToolTip(_("View in hex mode"));
  m_SyncMode->SetToolTip(_("Synchronize modified files"));
#endif

  m_HexMode->SetValue(
    wxConfigBase::Get()->ReadBool("HexMode", false)); // default no hex
    
  m_SyncMode->SetValue(i
    wxConfigBase::Get()->ReadBool("AllowSync", true));

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

  if (art.GetBitmap().IsOk())
  {
    return wxAuiToolBar::AddTool(
      toolId, 
      wxEmptyString,
      art.GetBitmap(wxART_TOOLBAR, GetToolBitmapSize()),
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
  case ID_EDIT_HEX_MODE:
    wxConfigBase::Get()->Write("HexMode", m_HexMode->GetValue());

    {
      auto* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        long flags = 0;
        if (m_HexMode->GetValue()) flags |= wxExSTC::STC_WIN_HEX;
        wxExFileDialog dlg(m_Frame, &stc->GetFile());
        if (dlg.ShowModalIfChanged() == wxID_CANCEL) return;
        stc->Open(stc->GetFileName(), 0, wxEmptyString, flags);
      }
    }
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
  AddControl(m_FindString);
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
  AddControl(m_IsRegularExpression);

  Realize();
}

void wxExFindToolBar::Initialize()
{
  m_FindString = new FindString(this, m_Frame);

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
    {
      auto* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        stc->FindNext(
          m_FindString->GetValue(), 
          wxExFindReplaceData::Get()->STCFlags(),
          (event.GetId() == wxID_DOWN));
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
  event.Enable(!m_FindString->GetValue().empty());
}

// Implementation of support class.

BEGIN_EVENT_TABLE(FindString, wxTextCtrl)
  EVT_MENU(wxID_DELETE, FindString::OnCommand)
  EVT_TEXT_ENTER(wxID_ANY, FindString::OnEnter)
END_EVENT_TABLE()

FindString::FindString(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, id, 
      wxExFindReplaceData::Get()->GetFindString(), pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);
}

void FindString::OnCommand(wxCommandEvent& event)
{
  // README: The delete key default behaviour does not delete the char right 
  // from insertion point.
  // Instead, the event is sent to the editor and a char is deleted from 
  // the editor. Therefore implement the delete here.
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

void FindString::OnEnter(wxCommandEvent& event)
{
  auto* stc = m_Frame->GetSTC();

  if (stc != NULL)
  {
    stc->FindNext(GetValue());
    wxExFindReplaceData::Get()->SetFindString(GetValue());
  }
}

#endif // wxUSE_AUI
#endif // wxUSE_GUI
