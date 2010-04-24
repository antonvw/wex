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
#include <wx/extension/stcfile.h>
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
// Offers a find combobox that allows you to find text
// on a current STC on an wxExFrame.
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
  , m_HexModeCheckBox(NULL)
  , m_SyncCheckBox(NULL)
{
}

void wxExToolBar::AddControls()
{
  AddTool(wxID_OPEN);
  AddTool(wxID_SAVE);
  AddTool(wxID_PRINT);
  AddSeparator();
  AddTool(wxID_FIND);
  
#ifdef __WXGTK__
  // wxID_EXECUTE is not part of art provider, but GTK directly,
  // so the following does not present a bitmap.
  AddSeparator();
  AddTool(wxID_EXECUTE);
#endif

  AddSeparator();

  AddControl(
    m_HexModeCheckBox = new wxCheckBox(
      this,
      ID_EDIT_HEX_MODE,
      "Hex"));

//  m_HexModeCheckBox->SetBackgroundColour(m_Frame->GetBackgroundColour());

//  AddTool(ID_SYNC_MODE, "Sync", wxNullBitmap, wxEmptyString, wxITEM_CHECK);
  AddControl(
    m_SyncCheckBox = new wxCheckBox(
      this,
      ID_SYNC_MODE,
      "Sync"));

#if wxUSE_TOOLTIPS
  m_HexModeCheckBox->SetToolTip(_("View in hex mode"));
  m_SyncCheckBox->SetToolTip(_("Synchronize modified files"));
#endif

  m_HexModeCheckBox->SetValue(wxConfigBase::Get()->ReadBool("HexMode", false)); // default no hex
  m_SyncCheckBox->SetValue(wxConfigBase::Get()->ReadBool("AllowSync", true));

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
    wxConfigBase::Get()->Write("HexMode", m_HexModeCheckBox->GetValue());

    {
      auto* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        long flags = 0;
        if (m_HexModeCheckBox->GetValue()) flags |= wxExSTCFile::STC_WIN_HEX;
        wxExFileDialog dlg(m_Frame, stc);
        if (dlg.ShowModalIfChanged() == wxID_CANCEL) return;
        stc->Open(stc->GetFileName(), 0, wxEmptyString, flags);
      }
    }
    break;

  case ID_SYNC_MODE:
    wxConfigBase::Get()->Write("AllowSync", m_SyncCheckBox->GetValue());
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
  ComboBox* cb = new ComboBox(this, 
    m_Frame, wxID_ANY, wxDefaultPosition, size);

  m_ComboBox = cb;

  m_MatchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, wxExFindReplaceData::Get()->GetTextMatchCase());

  m_MatchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, wxExFindReplaceData::Get()->GetTextMatchWholeWord());

  m_RegularExpression= new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, wxExFindReplaceData::Get()->GetTextRegEx());

  m_MatchCase->SetValue(wxExFindReplaceData::Get()->MatchCase());
  m_MatchWholeWord->SetValue(wxExFindReplaceData::Get()->MatchWord());
  m_RegularExpression->SetValue(wxExFindReplaceData::Get()->UseRegularExpression());
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
          m_ComboBox->GetValue(), 
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

// Implementation of support class.

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

  wxExComboBoxFromList(
    this, 
    wxExFindReplaceData::Get()->GetFindStrings());

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
  const auto key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    auto* stc = m_Frame->GetSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());

      wxExFindReplaceData::Get()->SetFindString(GetValue());

      Clear(); // so we can append again
      wxExComboBoxFromList(this, wxExFindReplaceData::Get()->GetFindStrings());
    }
  }
  else
  {
    event.Skip();
  }
}

#endif // wxUSE_AUI
#endif // wxUSE_GUI
