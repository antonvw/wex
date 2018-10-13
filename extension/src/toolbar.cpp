////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wex::toolbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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

namespace wex
{
  /// Support class.
  /// Offers a find text ctrl that allows you to find text
  /// on a current Grid, ListView or STC on an frame.
  /// Pressing key up and down browses through values from
  /// find_replace_data, and pressing enter sets value
  /// in find_replace_data.
  class find_textctrl : public wxTextCtrl
  {
  public:
    /// Constructor. Fills the text ctrl with value 
    /// from FindReplace from config.
    find_textctrl(frame* frame, const window_data& data);
      
    /// Finds current value in control.
    void Find(bool find_next = true, bool restore_position = false);
  private:
    frame* m_Frame;
  };
};

void FindPopupMenu(wxWindow* win, const std::list < std::string > & l, const wxPoint& pos)
{
  wxMenu* menu = new wxMenu();

  const int max_size = 25;
  int i = 0;
  
  for (const auto& it : l)
  {
    menu->Append(new wxMenuItem(menu, 
      wex::ID_FIND_FIRST + i++, 
      (it.size() >= max_size - 3 ? it.substr(0, max_size) + "..." : it)));
    
    if (i >= wex::FIND_MAX_FINDS) break;
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    menu->AppendSeparator();
    menu->Append(wex::ID_CLEAR_FINDS, wxGetStockLabel(wxID_CLEAR));
    win->PopupMenu(menu, pos);
  }
    
  delete menu;
}

wxPoint GetPoint(wxAuiToolBar* tb, wxAuiToolBarEvent& event)
{
  const wxRect rect = tb->GetToolRect(event.GetId());
  const wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
  return tb->ScreenToClient(pt);  
}
  
bool PrepDropDown(wxAuiToolBar* tb, wxAuiToolBarEvent& event)
{
  if (!event.IsDropDownClicked())
  {
    event.Skip();
    return false;
  }
  tb->SetToolSticky(event.GetId(), true);
  return true;
}
    
wex::toolbar::toolbar(managed_frame* frame, const window_data& data)
  : wxAuiToolBar(frame, data.Id(), data.Pos(), data.Size(), 
      data.Style() | wxAUI_TB_HORZ_TEXT | wxAUI_TB_PLAIN_BACKGROUND)
  , m_Frame(frame)
{
}

void wex::toolbar::AddControls(bool realize)
{
  AddTool(wxID_NEW);
  AddTool(wxID_OPEN);
  AddTool(wxID_SAVE);
  AddTool(wxID_PRINT);
  AddTool(wxID_UNDO);
  AddTool(wxID_REDO);
  AddTool(wxID_FIND);
  
  if (process::GetShell() != nullptr)
  {
    AddTool(wxID_EXECUTE);
  }

  SetToolDropDown(wxID_FIND, true);
  SetToolDropDown(wxID_OPEN, true);

  Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (!PrepDropDown(this, event)) return;
    FindPopupMenu(this, find_replace_data::Get()->GetFindStrings(), GetPoint(this, event));
    SetToolSticky(event.GetId(), false);}, wxID_FIND);
  
  Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (!PrepDropDown(this, event)) return;
    m_Frame->GetFileHistory().PopupMenu(this, ID_CLEAR_FILES, GetPoint(this, event));
    SetToolSticky(event.GetId(), false);}, wxID_OPEN);
      
  if (realize) Realize();
}

wxAuiToolBarItem* wex::toolbar::AddTool(
  int toolId,
  const wxString& label,
  const wxBitmap& bitmap,
  const wxString& shortHelp,
  wxItemKind kind)
{
  if (const stockart art(toolId); art.GetBitmap(wxART_TOOLBAR).IsOk())
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

  return nullptr;
}

wex::find_toolbar::find_toolbar(
  managed_frame* frame, const window_data& data)
  : toolbar(frame, data)
{
  const wxWindowID ID_MATCH_WHOLE_WORD = NewControlId();
  const wxWindowID ID_MATCH_CASE = NewControlId();
  const wxWindowID ID_REGULAR_EXPRESSION = NewControlId();

  find_textctrl* findCtrl = new find_textctrl(GetFrame(),
    window_data().Parent(this));
  wxCheckBox* matchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, find_replace_data::Get()->GetTextMatchCase());
  wxCheckBox* matchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, find_replace_data::Get()->GetTextMatchWholeWord());
  wxCheckBox* isRegularExpression = new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, find_replace_data::Get()->GetTextRegEx());

#if wxUSE_TOOLTIPS
  matchCase->SetToolTip(_("Search case sensitive"));
  matchWholeWord->SetToolTip(_("Search matching words"));
  isRegularExpression->SetToolTip(_("Search using regular expressions"));
#endif

  matchCase->SetValue(find_replace_data::Get()->MatchCase());
  matchWholeWord->SetValue(find_replace_data::Get()->MatchWord());
  isRegularExpression->SetValue(find_replace_data::Get()->UseRegEx());

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
    find_replace_data::Get()->SetMatchWord(
      matchWholeWord->GetValue());}, ID_MATCH_WHOLE_WORD);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    find_replace_data::Get()->SetMatchCase(
      matchCase->GetValue());}, ID_MATCH_CASE);
  Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
    find_replace_data::Get()->SetUseRegEx(
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

wex::options_toolbar::options_toolbar(
  managed_frame* frame, const window_data& data)
  : toolbar(frame, data)
{
}

void wex::options_toolbar::AddControls(bool realize)
{
  const wxWindowID ID_VIEW_PROCESS = NewControlId();
  
  // 0   1      2     3       4        5
  // id, label, name, config, tooltip, default
  for (const auto& it : std::vector<std::tuple<int, wxString, wxString, wxString, wxString, bool>> {
    {ID_VIEW_PROCESS, _("Process"), "PROCESS", "ViewProcess", _("View process"), false},
    {NewControlId(), "Hex", "HEX", "HexMode", _("Open in hex mode"), false},
    {NewControlId(), "Sync", "SYNC", "AllowSync", _("Synchronize modified files"), true}})
  {
    if (std::get<0>(it) == ID_VIEW_PROCESS && process::GetShell() == nullptr)
    {
      continue;
    }
    
    wxCheckBox* cb = new wxCheckBox(this, std::get<0>(it), std::get<1>(it));
    m_CheckBoxes.emplace_back(cb);
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

bool wex::options_toolbar::Update(const wxString& name, bool show)
{
  for (auto& it : m_CheckBoxes)
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

wex::find_textctrl::find_textctrl(
  frame* frame, const window_data& data)
  : wxTextCtrl(
      data.Parent(), 
      data.Id(),
      find_replace_data::Get()->GetFindString(), 
      data.Pos(), 
      data.Size(), 
      data.Style() | wxTE_PROCESS_ENTER)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);
  
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (!find_replace_data::Get()->m_FindStrings.Set(event.GetKeyCode(), this))
    {
      event.Skip();
    }});
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    if (auto* stc = m_Frame->GetSTC(); stc != nullptr)
    {
      stc->PositionSave();
    }
    event.Skip();});

#ifdef __WXOSX__      
  // FIXME. See also managed_frame.
  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    switch (event.GetKeyCode())
    {
      case WXK_RETURN:
        {
        wxCommandEvent event(wxEVT_TEXT_ENTER, m_windowId);
        event.SetEventObject( this );
        event.SetString( GetValue() );
        HandleWindowEvent(event);
        }
        break;
    }});
#endif
  
  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    Find(true, true);});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    event.Skip();
    if (!GetValue().empty())
    {
      find_replace_data::Get()->SetFindString(GetValue().ToStdString());
      Find();
    }});
}

void wex::find_textctrl::Find(bool find_next, bool restore_position)
{
  // We cannot use events here, as OnFindDialog in stc uses frd data,
  // whereas we need the GetValue here.
  if (auto* stc = m_Frame->GetSTC(); stc != nullptr)
  {
    if (restore_position)
    {
      stc->PositionRestore();
    }
    
    stc->FindNext(
      GetValue().ToStdString(), 
      -1,
      find_next);
  }
  else if (auto* grid = m_Frame->GetGrid(); grid != nullptr)
  {
    grid->FindNext(GetValue(), find_next);
  }
  else if (auto* lv = m_Frame->GetListView(); lv != nullptr)
  {
    lv->FindNext(GetValue().ToStdString(), find_next);
  }
}
