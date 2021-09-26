////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wex::toolbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/factory/defs.h>
#include <wex/factory/stc.h>
#include <wex/ui/art.h>
#include <wex/ui/bind.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/grid.h>
#include <wex/ui/listview.h>
#include <wex/ui/menu.h>
#include <wex/ui/textctrl.h>
#include <wex/ui/toolbar.h>

import<list>;

namespace wex
{
/// Support class.
/// Offers a find text ctrl that allows you to find text
/// on a current grid, listview or stc on an frame.
/// Pressing key up and down browses through values from
/// find_replace_data, and pressing enter sets value
/// in find_replace_data.
class find_textctrl : public textctrl
{
public:
  /// Constructor. Fills the textctrl with value
  /// from find_replace_data.
  find_textctrl(wex::frame* frame, const data::window& data);

  /// Finds current value in control.
  void find(bool find_next = true, bool restore_position = false);
};

void find_popup_menu(
  wxWindow*                       win,
  const textctrl_input::values_t& l,
  const wxPoint&                  pos)
{
  auto* menu = new wex::menu();

  const int max_size = 25;
  int       i        = 0;

  for (const auto& it : l)
  {
    menu->append(
      {{wex::ID_FIND_FIRST + i++,
        (it.size() >= max_size - 3 ? it.substr(0, max_size) + "..." : it)}});

    if (i >= wex::FIND_MAX_FINDS)
      break;
  }

  if (menu->GetMenuItemCount() > 0)
  {
    menu->append({{}, {wex::ID_CLEAR_FINDS, wxGetStockLabel(wxID_CLEAR)}});
    win->PopupMenu(menu, pos);
  }

  delete menu;
}

wxPoint get_point(wxAuiToolBar* tb, wxAuiToolBarEvent& event)
{
  const wxRect  rect = tb->GetToolRect(event.GetId());
  const wxPoint pt   = tb->ClientToScreen(rect.GetBottomLeft());
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
}; // namespace wex

wex::toolbar::toolbar(wex::frame* frame, const data::window& data)
  : wxAuiToolBar(frame, data.id(), data.pos(), data.size(), data.style())
  , m_frame(frame)
{
}

void wex::toolbar::add_checkboxes(const checkboxes_t& v, bool realize)
{
  // 0   1      2     3       4        5        6
  // id, label, name, config, tooltip, default, lambda
  for (const auto& it : v)
  {
    auto* cb = new wxCheckBox(this, std::get<0>(it), std::get<1>(it));

    cb->SetToolTip(std::get<4>(it));
    cb->SetValue(config(std::get<3>(it)).get(std::get<5>(it)));
    cb->SetName(std::get<2>(it));

    m_checkboxes.emplace_back(cb);

    AddControl(cb);

    Bind(
      wxEVT_CHECKBOX,
      [=, this](wxCommandEvent& event)
      {
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
          m_frame->pane_show("PROCESS", cb->GetValue());
        };
      },
      std::get<0>(it));
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
      _("Process"),
      "PROCESS",
      "ViewProcess",
      _("View process"),
      false,
      nullptr},
     {NewControlId(),
      "Hex",
      "HEX",
      "is_hexmode",
      _("Open in hex mode"),
      false,
      nullptr},
     {NewControlId(),
      "Sync",
      "SYNC",
      "AllowSync",
      _("Synchronize modified files"),
      true,
      nullptr}},
    realize);
}

void wex::toolbar::add_find(bool realize)
{
  auto* findCtrl = new find_textctrl(m_frame, data::window().parent(this));

  AddControl(findCtrl->control());

  add_tool(
    {data::toolbar_item(wxID_DOWN)
       .bitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR))
       .help(_("Find next")),
     data::toolbar_item(wxID_UP)
       .bitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR))
       .help(_("Find previous"))});

  add_checkboxes(
    {{NewControlId(),
      after(find_replace_data::get()->text_match_word(), '.', false),
      "",
      "",
      _("Search matching words"),
      find_replace_data::get()->match_word(),
      [](wxCheckBox* cb)
      {
        find_replace_data::get()->set_match_word(cb->GetValue());
      }},
     {NewControlId(),
      after(find_replace_data::get()->text_match_case(), '.', false),
      "",
      "",
      _("Search case sensitive"),
      find_replace_data::get()->match_case(),
      [](wxCheckBox* cb)
      {
        find_replace_data::get()->set_match_case(cb->GetValue());
      }},
     {NewControlId(),
      after(find_replace_data::get()->text_regex(), '.', false),
      "",
      "",
      _("Search using regular expressions"),
      find_replace_data::get()->is_regex(),
      [](wxCheckBox* cb)
      {
        find_replace_data::get()->set_regex(cb->GetValue());
      }}},
    false);

  if (realize)
  {
    Realize();
  }

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        findCtrl->find(true);
      },
      wxID_DOWN},
     {[=, this](wxCommandEvent& event)
      {
        findCtrl->find(false);
      },
      wxID_UP}});

  bind(this).ui(
    {{[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(!findCtrl->get_text().empty());
      },
      wxID_DOWN},
     {[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(!findCtrl->get_text().empty());
      },
      wxID_UP}});
}

void wex::toolbar::add_standard(bool realize)
{
  add_tool(
    {{wxID_NEW},
     {wxID_OPEN},
     {wxID_SAVE},
     {wxID_PRINT},
     {wxID_UNDO},
     {wxID_REDO},
     {wxID_FIND},
     {wxID_EXECUTE}},
    false);

  SetToolDropDown(wxID_FIND, true);
  SetToolDropDown(wxID_OPEN, true);

  Bind(
    wxEVT_AUITOOLBAR_TOOL_DROPDOWN,
    [=, this](wxAuiToolBarEvent& event)
    {
      if (!prep_dropdown(this, event))
        return;

      find_popup_menu(
        this,
        find_replace_data::get()->get_find_strings(),
        get_point(this, event));

      SetToolSticky(event.GetId(), false);
    },
    wxID_FIND);

  Bind(
    wxEVT_AUITOOLBAR_TOOL_DROPDOWN,
    [=, this](wxAuiToolBarEvent& event)
    {
      if (!prep_dropdown(this, event))
        return;

      m_frame->file_history().popup_menu(
        this,
        ID_CLEAR_FILES,
        get_point(this, event));

      SetToolSticky(event.GetId(), false);
    },
    wxID_OPEN);

  if (realize)
  {
    Realize();
  }
}

bool wex::toolbar::add_tool(
  const std::vector<data::toolbar_item> v,
  bool                                  realize)
{
  for (const auto& it : v)
  {
    if (const stockart art(it.id()); art.get_bitmap(wxART_TOOLBAR).IsOk())
    {
      if (!AddTool(
            it.id(),
            wxEmptyString, // no label
#ifdef __WXGTK__
            art.get_bitmap(wxART_TOOLBAR),
#else
            art.get_bitmap(wxART_MENU, wxSize(16, 16)),
#endif
            wxGetStockLabel(it.id(), wxSTOCK_NOFLAGS), // short help
            it.kind()))
      {
        return false;
      }
    }
    else if (it.bitmap().IsOk())
    {
      if (!AddTool(it.id(), it.label(), it.bitmap(), it.help(), it.kind()))
      {
        return false;
      }
    }
  }

  if (realize)
  {
    Realize();
  }

  return true;
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

wex::find_textctrl::find_textctrl(wex::frame* mng, const data::window& data)
  : textctrl(mng, find_replace_data::get()->get_find_string(), data)
{
  mng->bind_accelerators(
    control(),
    {{wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE}});

  control()->Bind(
    wxEVT_CHAR,
    [=, this](wxKeyEvent& event)
    {
      if (!find_replace_data::get()->m_find_strings.set(
            event.GetKeyCode(),
            this))
      {
        event.Skip();
      }
    });

  control()->Bind(
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      if (auto* stc = get_frame()->get_stc(); stc != nullptr)
      {
        stc->position_save();
      }
      event.Skip();
    });

  control()->Bind(
    wxEVT_TEXT,
    [=, this](wxCommandEvent& event)
    {
      event.Skip();
      find(true, true);
    });

  control()->Bind(
    wxEVT_TEXT_ENTER,
    [=, this](wxCommandEvent& event)
    {
      event.Skip();
      if (!get_text().empty())
      {
        find_replace_data::get()->set_find_string(get_text());
        find();
      }
    });
}

void wex::find_textctrl::find(bool find_next, bool restore_position)
{
  if (auto* stc = get_frame()->get_stc(); stc != nullptr)
  {
    if (restore_position)
    {
      stc->position_restore();
    }

    stc->find(get_text(), -1, find_next);
  }
  else if (auto* grid = dynamic_cast<wex::grid*>(get_frame()->get_grid());
           grid != nullptr)
  {
    grid->find_next(get_text(), find_next);
  }
  else if (auto* lv = get_frame()->get_listview(); lv != nullptr)
  {
    lv->find_next(get_text(), find_next);
  }
}
