////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wxExToolBar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/art.h>
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
//#include <wx/extension/util.h>

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
class wxExTextCtrl : public wxExFindTextCtrl
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
private:
  wxExFrame* m_Frame;
};

wxExToolBar::wxExToolBar(wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiToolBar(frame, id, pos, size, 
    style | wxAUI_TB_HORZ_TEXT)
  , m_Frame(frame)
{
}

void wxExToolBar::AddControls()
{
  AddTool(wxID_NEW);
  AddTool(wxID_OPEN);
  AddTool(wxID_SAVE);
  AddTool(wxID_PRINT);
  AddTool(wxID_FIND);

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
#ifdef __WXGTK__
      art.GetBitmap(wxART_TOOLBAR),
#else
      art.GetBitmap(wxART_MENU, wxSize(16, 16)),
#endif
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

wxExFindToolBar::wxExFindToolBar(
  wxExManagedFrame* frame,
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
  
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetMatchWord(
      m_MatchWholeWord->GetValue());}, ID_MATCH_WHOLE_WORD);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetMatchCase(
      m_MatchCase->GetValue());}, ID_MATCH_CASE);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetUseRegEx(
      m_IsRegularExpression->GetValue());}, ID_REGULAR_EXPRESSION);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FindCtrl->Find(true);}, wxID_DOWN);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FindCtrl->Find(false);}, wxID_UP);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!m_FindCtrl->GetValue().empty());}, wxID_DOWN);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!m_FindCtrl->GetValue().empty());}, wxID_UP);
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
  m_IsRegularExpression->SetValue(wxExFindReplaceData::Get()->UseRegEx());
}

wxExOptionsToolBar::wxExOptionsToolBar(wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExToolBar(frame, id, pos, size, style)
  , m_HexMode(new wxCheckBox(
    this,
    ID_HEX_MODE,
    "Hex"))
  , m_SyncMode(new wxCheckBox(
    this,
    ID_SYNC_MODE,
    "Sync"))
{
  AddControl(m_HexMode);
  AddControl(m_SyncMode);

#if wxUSE_TOOLTIPS
  m_HexMode->SetToolTip(_("Open in hex mode"));
  m_SyncMode->SetToolTip(_("Synchronize modified files"));
#endif

  m_HexMode->SetValue(wxConfigBase::Get()->ReadBool("HexMode", false));
  m_SyncMode->SetValue(wxConfigBase::Get()->ReadBool("AllowSync", true));
  
  Realize();
  
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxConfigBase::Get()->Write("HexMode", m_HexMode->GetValue());}, ID_HEX_MODE);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxConfigBase::Get()->Write("AllowSync", m_SyncMode->GetValue());
    GetFrame()->SyncAll();}, ID_SYNC_MODE);
}

// Implementation of support class.

wxExTextCtrl::wxExTextCtrl(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxExFindTextCtrl(parent, id, pos, size)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    wxExSTC* stc = m_Frame->GetSTC();
    if (stc != NULL)
    {
      stc->PositionSave();
    }
    event.Skip();});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    Find(true, true);});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    event.Skip();
    if (!GetValue().empty())
    {
      Find();
    }});
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
      -1,
      find_next);
  }
}
#endif // wxUSE_GUI
