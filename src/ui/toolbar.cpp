////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wex::toolbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/syntax/stc.h>
#include <wex/ui/art.h>
#include <wex/ui/ex-commandline-input.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/menu.h>
#include <wex/ui/toolbar.h>
#include <wx/checkbox.h>
#include <wx/stockitem.h>

#include "findbar.h"

namespace wex
{
void find_popup_menu(
  wxWindow*                             win,
  const ex_commandline_input::values_t& l,
  const wxPoint&                        pos)
{
  auto*     menu     = new wex::menu();
  const int max_size = 25;

  for (int i = 0; const auto& it : l)
  {
    menu->append(
      {{wex::ID_FIND_FIRST + i++,
        (it.size() >= max_size - 3 ? it.substr(0, max_size) + "..." : it)}});

    if (i >= wex::FIND_MAX_FINDS)
    {
      break;
    }
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
  const auto& rect = tb->GetToolRect(event.GetId());
  const auto& pt   = tb->ClientToScreen(rect.GetBottomLeft());
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
  m_find_bar = new find_bar(m_frame, data::window().parent(this));

  AddControl(m_find_bar->control());

  add_tool(
    {data::toolbar_item(wxID_DOWN)
       .bitmap(wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR))
       .help(_("Find next")),
     data::toolbar_item(wxID_UP)
       .bitmap(wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR))
       .help(_("Find previous"))});

  add_checkboxes(
    {{NewControlId(),
      rfind_after(find_replace_data::get()->text_match_word(), "."),
      "",
      "",
      _("Search matching words"),
      find_replace_data::get()->match_word(),
      [](wxCheckBox* cb)
      {
        find_replace_data::get()->set_match_word(cb->GetValue());
      }},
     {NewControlId(),
      rfind_after(find_replace_data::get()->text_match_case(), "."),
      "",
      "",
      _("Search case sensitive"),
      find_replace_data::get()->match_case(),
      [](wxCheckBox* cb)
      {
        find_replace_data::get()->set_match_case(cb->GetValue());
      }},
     {NewControlId(),
      rfind_after(find_replace_data::get()->text_regex(), "."),
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
    {{[=, this](const wxCommandEvent& event)
      {
        m_find_bar->find(true);
      },
      wxID_DOWN},
     {[=, this](const wxCommandEvent& event)
      {
        m_find_bar->find(false);
      },
      wxID_UP}});

  bind(this).ui(
    {{[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(!m_find_bar->get_text().empty());
      },
      wxID_DOWN},
     {[=, this](wxUpdateUIEvent& event)
      {
        event.Enable(!m_find_bar->get_text().empty());
      },
      wxID_UP}});
}

void wex::toolbar::add_standard(bool realize)
{
  add_tool(
    {{wxID_NEW},
     {wxID_OPEN},
     {wxID_BACKWARD},
     {wxID_FORWARD},
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
      {
        return;
      }

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
      {
        return;
      }

      m_frame->file_history().popup_menu(
        this,
        ID_CLEAR_FILES,
        get_point(this, event));

      SetToolSticky(event.GetId(), false);
    },
    wxID_OPEN);

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        m_frame->browse(event);
      },
      wxID_FORWARD,
      wxID_BACKWARD}});

  if (realize)
  {
    Realize();
  }
}

bool wex::toolbar::add_tool(
  const std::vector<data::toolbar_item>& v,
  bool                                   realize)
{
  if (!std::all_of(
        v.begin(),
        v.end(),
        [this](const auto& it)
        {
          // If the id has a bitmap from our art.
          if (const art art(it.id()); art.get_bitmap(wxART_TOOLBAR).IsOk())
          {
            if (!AddTool(
                  it.id(),
                  wxEmptyString, // no label
#ifdef __WXGTK__
                  art.get_bitmap(wxART_TOOLBAR),
#else
                  art.get_bitmap(wxART_MENU, wxSize(16, 16)),
#endif
                  wxIsStockID(it.id()) ?
                    wxGetStockLabel(it.id(), wxSTOCK_NOFLAGS).ToStdString() :
                    it.help(),
                  it.kind()))
            {
              return false;
            }
          }
          // If the it has a bitmap on it's own.
          else if (it.bitmap().IsOk())
          {
            if (!AddTool(
                  it.id(),
                  it.label(),
                  it.bitmap(),
                  it.help(),
                  it.kind()))
            {
              return false;
            }
          }
          return true;
        }))
  {
    return false;
  }

  if (realize)
  {
    Realize();
  }

  return true;
}

bool wex::toolbar::Destroy()
{
  delete m_find_bar;

  return wxAuiToolBar::Destroy();
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
