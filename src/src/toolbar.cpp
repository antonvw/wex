////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wex::toolbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <tuple>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/toolbar.h>
#include <wex/accelerators.h>
#include <wex/config.h>
#include <wex/art.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/grid.h>
#include <wex/listview.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/util.h>

namespace wex
{
  /// Support class.
  /// Offers a find text ctrl that allows you to find text
  /// on a current grid, listview or stc on an frame.
  /// Pressing key up and down browses through values from
  /// find_replace_data, and pressing enter sets value
  /// in find_replace_data.
  class find_textctrl : public wxTextCtrl
  {
  public:
    /// Constructor. Fills the textctrl with value 
    /// from find_replace_data.
    find_textctrl(frame* frame, const window_data& data);
      
    /// Finds current value in control.
    void find(bool find_next = true, bool restore_position = false);
  private:
    frame* m_frame;
  };

  void find_popup_menu(
    wxWindow* win, 
    const std::list < std::string > & l, 
    const wxPoint& pos)
  {
    auto* menu = new wex::menu();

    const int max_size = 25;
    int i = 0;
    
    for (const auto& it : l)
    {
      menu->append({
        {wex::ID_FIND_FIRST + i++, 
         (it.size() >= max_size - 3 ? it.substr(0, max_size) + "..." : it)}});
      
      if (i >= wex::FIND_MAX_FINDS) break;
    }
    
    if (menu->GetMenuItemCount() > 0)
    {
      menu->append({{},
        {wex::ID_CLEAR_FINDS, wxGetStockLabel(wxID_CLEAR)}});
      win->PopupMenu(menu, pos);
    }
      
    delete menu;
  }

  wxPoint get_point(wxAuiToolBar* tb, wxAuiToolBarEvent& event)
  {
    const wxRect rect = tb->GetToolRect(event.GetId());
    const wxPoint pt = tb->ClientToScreen(rect.GetBottomLeft());
    return tb->ScreenToClient(pt);  
  }
    
  bool prep_dropdown(wxAuiToolBar* tb, wxAuiToolBarEvent& event)
  {
    if (!event.IsDropDownClicked())
    {
      event.Skip();
      return false;
    }

    tb->SetToolSticky(event.GetId(), true);
    return true;
  }
  
  const wxWindowID ID_VIEW_PROCESS = wxWindowBase::NewControlId();
};
    
wex::toolbar::toolbar(managed_frame* frame, const window_data& data)
  : wxAuiToolBar(frame, 
      data.id(), 
      data.pos(), 
      data.size(), 
      data.style() | wxAUI_TB_HORZ_TEXT | wxAUI_TB_PLAIN_BACKGROUND)
  , m_frame(frame)
{
}

void wex::toolbar::add_checkboxes(
  const checkboxes_t& v, bool realize)
{
  // 0   1      2     3       4        5        6
  // id, label, name, config, tooltip, default, lambda
  for (const auto& it : v)
  {
    if (std::get<0>(it) == ID_VIEW_PROCESS && process::get_shell() == nullptr)
    {
      continue;
    }
    
    auto* cb = new wxCheckBox(this, std::get<0>(it), std::get<1>(it));

    cb->SetToolTip(std::get<4>(it));
    cb->SetValue(config(std::get<3>(it)).get(std::get<5>(it)));
    cb->SetName(std::get<2>(it));

    m_checkboxes.emplace_back(cb);

    AddControl(cb);

    Bind(wxEVT_CHECKBOX, [=](wxCommandEvent& event) {
      if (std::get<6>(it) == nullptr)
      {
        config(std::get<3>(it)).set(cb->GetValue());
      }
      else
      {
        std::get<6>(it)(cb);
      }

      if (event.GetId() == ID_VIEW_PROCESS)
      {
        m_frame->show_pane("PROCESS", cb->GetValue());
      };}, std::get<0>(it));
  }

  if (realize)
  {
    Realize();
  }
}

void wex::toolbar::add_checkboxes_standard(bool realize)
{
  add_checkboxes(
    {{ID_VIEW_PROCESS, 
       _("Process"), "PROCESS", "ViewProcess", _("View process"), false, nullptr},
     {NewControlId(), 
       "Hex", "HEX", "is_hexmode", _("Open in hex mode"), false, nullptr},
     {NewControlId(), 
       "Sync", "SYNC", "AllowSync", _("Synchronize modified files"), true, nullptr}},
    realize);
}

void wex::toolbar::add_find(bool realize)
{
  auto* findCtrl = new find_textctrl(m_frame,
    window_data().parent(this));

  AddControl(findCtrl);

  add_tool(
    wxID_DOWN, 
    std::string(), 
    wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR),
    _("Find next"));

  add_tool(
    wxID_UP, 
    std::string(), 
    wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR),
    _("Find previous"));

  add_checkboxes({
    {NewControlId(), 
     after(find_replace_data::get()->text_match_word(), '.', false), 
     "", 
     "", 
     _("Search matching words"), 
     find_replace_data::get()->match_word(),
     [](wxCheckBox* cb) {
       find_replace_data::get()->set_match_word(cb->GetValue());}},
    {NewControlId(), 
     after(find_replace_data::get()->text_match_case(), '.', false), 
     "", 
     "", 
     _("Search case sensitive"), 
     find_replace_data::get()->match_case(),
     [](wxCheckBox* cb) {
       find_replace_data::get()->set_match_case(cb->GetValue());}},
    {NewControlId(), 
     after(find_replace_data::get()->text_regex(), '.', false), 
     "", 
     "", 
     _("Search using regular expressions"), 
     find_replace_data::get()->use_regex(),
     [](wxCheckBox* cb) {
       find_replace_data::get()->set_use_regex(cb->GetValue());}}},
    false);

  if (realize)
  {
    Realize();
  }
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    findCtrl->find(true);}, wxID_DOWN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    findCtrl->find(false);}, wxID_UP);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!findCtrl->GetValue().empty());}, wxID_DOWN);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!findCtrl->GetValue().empty());}, wxID_UP);
}

void wex::toolbar::add_standard(bool realize)
{
  add_tool(wxID_NEW);
  add_tool(wxID_OPEN);
  add_tool(wxID_SAVE);
  add_tool(wxID_PRINT);
  add_tool(wxID_UNDO);
  add_tool(wxID_REDO);
  add_tool(wxID_FIND);
  
  if (process::get_shell() != nullptr)
  {
    add_tool(wxID_EXECUTE);
  }

  SetToolDropDown(wxID_FIND, true);
  SetToolDropDown(wxID_OPEN, true);

  Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (!prep_dropdown(this, event)) return;

    find_popup_menu(
      this, 
      find_replace_data::get()->get_find_strings(), 
      get_point(this, event));

    SetToolSticky(event.GetId(), false);}, wxID_FIND);
  
  Bind(wxEVT_AUITOOLBAR_TOOL_DROPDOWN, [=](wxAuiToolBarEvent& event) {
    if (!prep_dropdown(this, event)) return;

    m_frame->file_history().popup_menu(
      this, 
      ID_CLEAR_FILES, 
      get_point(this, event));
  
    SetToolSticky(event.GetId(), false);}, wxID_OPEN);
      
  if (realize) 
  {
    Realize();
  }
}

wxAuiToolBarItem* wex::toolbar::add_tool(
  int toolId,
  const std::string& label,
  const wxBitmap& bitmap,
  const std::string& shortHelp,
  wxItemKind kind)
{
  if (const stockart art(toolId); art.get_bitmap(wxART_TOOLBAR).IsOk())
  {
    return AddTool(
      toolId, 
      wxEmptyString, // no label
#ifdef __WXGTK__
      art.get_bitmap(wxART_TOOLBAR),
#else
      art.get_bitmap(wxART_MENU, wxSize(16, 16)),
#endif
      wxGetStockLabel(toolId, wxSTOCK_NOFLAGS), // short help
      kind);
  }
  else if (bitmap.IsOk())
  {
    return AddTool(
      toolId, 
      label,
      bitmap,
      shortHelp,
      kind);
  }

  return nullptr;
}

bool wex::toolbar::set_checkbox(const std::string& name, bool show) const
{
  for (auto& it : m_checkboxes)
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
      data.parent(), 
      data.id(),
      find_replace_data::get()->get_find_string(), 
      data.pos(), 
      data.size(), 
      data.style() | wxTE_PROCESS_ENTER)
  , m_frame(frame)
{
  accelerators({
   {wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE, nullptr}}).set(this);
  
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (!find_replace_data::get()->m_find_strings.set(event.GetKeyCode(), this))
    {
      event.Skip();
    }});
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    if (auto* stc = m_frame->get_stc(); stc != nullptr)
    {
      stc->position_save();
    }
    event.Skip();});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    find(true, true);});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    event.Skip();
    if (!GetValue().empty())
    {
      find_replace_data::get()->set_find_string(GetValue());
      find();
    }});
}

void wex::find_textctrl::find(bool find_next, bool restore_position)
{
  // We cannot use events here, as OnFindDialog in stc uses frd data,
  // whereas we need the GetValue here.
  if (auto* stc = m_frame->get_stc(); stc != nullptr)
  {
    if (restore_position)
    {
      stc->position_restore();
    }
    
    stc->find_next(
      GetValue(), 
      -1,
      find_next);
  }
  else if (auto* grid = m_frame->get_grid(); grid != nullptr)
  {
    grid->find_next(GetValue(), find_next);
  }
  else if (auto* lv = m_frame->get_listview(); lv != nullptr)
  {
    lv->find_next(GetValue(), find_next);
  }
}
