////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wxExToolBar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <tuple>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/art.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

// Support class.
// Offers a find text ctrl that allows you to find text
// on a current Grid, ListView or STC on an wxExFrame.
class FindTextCtrl : public wxExFindTextCtrl
{
public:
  /// Constructor. Fills the text ctrl with value 
  /// from FindReplace from config.
  FindTextCtrl(
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

void FindPopupMenu(wxWindow* win,
  const std::list < wxString > & l, int first_id, const wxPoint& pos)
{
  wxMenu* menu = new wxMenu();

  int i = 0;
  const int max_size = 25;
  
  for (const auto& it : l)
  {
    const wxString label = 
      (it.size() >= max_size - 3 ? it.Left(max_size) + "..." : it);
    wxMenuItem* item = new wxMenuItem(
      menu, 
      first_id + i++, 
      label);

    menu->Append(item);
    
    if (i >= FIND_MAX_FINDS) break;
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    menu->AppendSeparator();
    menu->Append(ID_CLEAR_FINDS, wxGetStockLabel(wxID_CLEAR));
      
    win->PopupMenu(menu, pos);
  }
    
  delete menu;
}
  
wxExToolBar::wxExToolBar(wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiToolBar(frame, id, pos, size, 
    style | wxAUI_TB_HORZ_TEXT | wxAUI_TB_PLAIN_BACKGROUND)
  , m_Frame(frame)
{
}

void wxExToolBar::AddControls(bool realize)
{
  AddTool(wxID_NEW);
  AddTool(wxID_OPEN);
  AddTool(wxID_SAVE);
  AddTool(wxID_PRINT);
  AddTool(wxID_FIND);
  
  if (wxExProcess::GetShell() != NULL)
  {
    AddTool(wxID_EXECUTE);
  }

  SetToolDropDown(wxID_FIND, true);
  SetToolDropDown(wxID_OPEN, true);
  
  m_Frame->Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (event.IsDropDownClicked())
    {
      wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(event.GetEventObject());
  
      tb->SetToolSticky(event.GetId(), true);
  
      // create the popup menu
      // line up our menu with the button
      wxRect rect = tb->GetToolRect(event.GetId());
      wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
      pt = ScreenToClient(pt);
      
      FindPopupMenu(this, wxExFindReplaceData::Get()->GetFindStrings(), ID_FIND_FIRST, pt);
  
      // make sure the button is "un-stuck"
      tb->SetToolSticky(event.GetId(), false);
    }
    else
    {
      event.Skip();
  }}, wxID_FIND);
  
  Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (event.IsDropDownClicked())
    {
      wxAuiToolBar* tb = static_cast<wxAuiToolBar*>(event.GetEventObject());
  
      tb->SetToolSticky(event.GetId(), true);
  
      // create the popup menu
      // line up our menu with the button
      wxRect rect = tb->GetToolRect(event.GetId());
      wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
      pt = ScreenToClient(pt);

      m_Frame->GetFileHistory().PopupMenu(this, ID_CLEAR_FILES, pt);
  
      // make sure the button is "un-stuck"
      tb->SetToolSticky(event.GetId(), false);
    }
    else
    {
      event.Skip();
    }}, wxID_OPEN);
    
  if (realize) Realize();
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
      wxEmptyString, // no label
#ifdef __WXGTK__
      art.GetBitmap(wxART_TOOLBAR),
#else
      art.GetBitmap(wxART_MENU, wxSize(16, 16)),
#endif
      wxGetStockLabel(toolId, wxSTOCK_NOFLAGS), // short help
      kind);
  }
  else if (bitmap.IsOk())
  {
    return wxAuiToolBar::AddTool(
      toolId, 
      label,
      bitmap,
      shortHelp,
      kind);
  }

  return NULL;
}

wxExFindToolBar::wxExFindToolBar(
  wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExToolBar(frame, id, pos, size, style)
{
  const wxWindowID ID_MATCH_WHOLE_WORD = NewControlId();
  const wxWindowID ID_MATCH_CASE = NewControlId();
  const wxWindowID ID_REGULAR_EXPRESSION = NewControlId();

  FindTextCtrl* findCtrl = new FindTextCtrl(this, GetFrame());
  wxCheckBox* matchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, wxExFindReplaceData::Get()->GetTextMatchCase());
  wxCheckBox* matchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, wxExFindReplaceData::Get()->GetTextMatchWholeWord());
  wxCheckBox* isRegularExpression = new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, wxExFindReplaceData::Get()->GetTextRegEx());

#if wxUSE_TOOLTIPS
  matchCase->SetToolTip(_("Search case sensitive"));
  matchWholeWord->SetToolTip(_("Search matching words"));
  isRegularExpression->SetToolTip(_("Search using regular expressions"));
#endif

  matchCase->SetValue(wxExFindReplaceData::Get()->MatchCase());
  matchWholeWord->SetValue(wxExFindReplaceData::Get()->MatchWord());
  isRegularExpression->SetValue(wxExFindReplaceData::Get()->UseRegEx());

  // And place the controls on the toolbar.
  AddControl(findCtrl);

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

  AddControl(matchWholeWord);
  AddControl(matchCase);
  AddControl(isRegularExpression);

  Realize();
  
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetMatchWord(
      matchWholeWord->GetValue());}, ID_MATCH_WHOLE_WORD);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetMatchCase(
      matchCase->GetValue());}, ID_MATCH_CASE);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetUseRegEx(
      isRegularExpression->GetValue());}, ID_REGULAR_EXPRESSION);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    findCtrl->Find(true);}, wxID_DOWN);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    findCtrl->Find(false);}, wxID_UP);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!findCtrl->GetValue().empty());}, wxID_DOWN);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!findCtrl->GetValue().empty());}, wxID_UP);
}

wxExOptionsToolBar::wxExOptionsToolBar(wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExToolBar(frame, id, pos, size, style)
{
}

void wxExOptionsToolBar::AddControls(bool realize)
{
  const wxWindowID ID_VIEW_PROCESS = NewControlId();
  
  // 0   1      2     3       4        5
  // id, label, name, config, tooltip, default
  for (auto it : std::vector<std::tuple<int, wxString, wxString, wxString, wxString, bool>> {
    std::make_tuple(ID_VIEW_PROCESS, _("Process"), "PROCESS", "ViewProcess", _("View Process"), false),
    std::make_tuple(NewControlId(), "Hex", "HEX", "HexMode", _("Open in hex mode"), false),
    std::make_tuple(NewControlId(), "Sync", "SYNC", "AllowSync", _("Synchronize modified files"), true)})
  {
    if (std::get<0>(it) == ID_VIEW_PROCESS && wxExProcess::GetShell() == NULL)
    {
      continue;
    }
    
    wxCheckBox* cb = new wxCheckBox(this, std::get<0>(it), std::get<1>(it));
    m_CheckBoxes.push_back(cb);
#if wxUSE_TOOLTIPS
    cb->SetToolTip(std::get<4>(it));
#endif
    cb->SetValue(wxConfigBase::Get()->ReadBool(std::get<3>(it), std::get<5>(it)));
    cb->SetName(std::get<2>(it));
    AddControl(cb);
    Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
      wxConfigBase::Get()->Write(std::get<3>(it), cb->GetValue());
      if (event.GetId() == ID_VIEW_PROCESS)
      {
        GetFrame()->ShowPane("PROCESS", cb->GetValue());};}, std::get<0>(it));
  }

  if (realize)
  {
    Realize();
  }
}

bool wxExOptionsToolBar::Update(const wxString& name, bool show)
{
  for (auto it : m_CheckBoxes)
  {
    if (it->GetName() == name)
    {
      it->SetValue(show);
      return true;
    }
  }
  
  return false;
}

// Implementation of support class.

FindTextCtrl::FindTextCtrl(
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

void FindTextCtrl::Find(bool find_next, bool restore_position)
{
  // We cannot use events here, as OnFindDialog in stc uses frd data,
  // whereas we need the GetValue here.
  wxExSTC* stc = m_Frame->GetSTC();
  wxExGrid* grid = m_Frame->GetGrid();
  wxExListView* lv = m_Frame->GetListView();

  if (stc != NULL)
  {
    if (restore_position)
    {
      stc->PositionRestore();
    }
    
    stc->FindNext(
      GetValue(), 
      -1,
      find_next);
  }
  else if (grid != NULL)
  {
    grid->FindNext(GetValue(), find_next);
  }
  else if (lv != NULL)
  {
    lv->FindNext(GetValue(), find_next);
  }
}
#endif // wxUSE_GUI
