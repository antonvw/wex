////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/tool.h>
#include <wex/core/core.h>
#include <wex/factory/listview.h>
#include <wex/factory/stc.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/ui/frame.h>
#include <wex/ui/menu.h>
#include <wx/app.h>

#define PRINT_COMPONENT(ID, ACTION)                                            \
  {ID,                                                                         \
   "",                                                                         \
   data::menu().action(                                                        \
     [=, this](const wxCommandEvent& event)                                    \
     {                                                                         \
       if (auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());  \
           frame->get_stc() != nullptr)                                        \
       {                                                                       \
         frame->get_stc()->ACTION();                                           \
       }                                                                       \
       else if (frame->get_listview() != nullptr)                              \
       {                                                                       \
         frame->get_listview()->ACTION();                                      \
       }                                                                       \
     })}

wex::menu::menu(menu_t style, const menu_items_t& items)
  : m_style(style)
{
  append(items);
}

wex::menu::menu(const menu_items_t& items, menu_t style)
  : m_style(style)
{
  append(items);
}

wex::menu::menu(const std::string& title, menu_t style)
  : wxMenu(title)
  , m_style(style)
{
}

size_t wex::menu::append(const menu_items_t& items)
{
  const auto count(GetMenuItemCount());

  for (const auto& item : items)
  {
    switch (item.type())
    {
      case menu_item::EDIT:
        append_edit();
        break;

      case menu_item::EDIT_INVERT:
        append_edit(true);
        break;

      case menu_item::EXIT:
        append(
          {{wxID_EXIT,
            "",
            data::menu().action(
              [=, this](const wxCommandEvent& event)
              {
                wxTheApp->GetTopWindow()->Close(true);
              })}});
        break;

      case menu_item::PRINT:
        append_print();
        break;

      case menu_item::SEPARATOR:
        append_separator();
        break;

      default:
        item.append(this);
    }
  }

  return GetMenuItemCount() - count;
}

void wex::menu::append_edit(bool add_invert)
{
  if (!m_style[IS_READ_ONLY] && m_style[IS_SELECTED])
  {
    append({{wxID_CUT}});
  }

  if (m_style[IS_SELECTED])
  {
    append({{wxID_COPY}});
  }

  if (!m_style[IS_READ_ONLY] && m_style[CAN_PASTE])
  {
    append({{wxID_PASTE}});
  }

  if (!m_style[IS_SELECTED] && !m_style[IS_EMPTY])
  {
    append({{wxID_SELECTALL}});
  }
  else
  {
    if (add_invert && !m_style[IS_EMPTY])
    {
      append({{ID_EDIT_SELECT_NONE, _("&Deselect All")}});
    }
  }

  if (m_style[ALLOW_CLEAR])
  {
    append({{wxID_CLEAR}});
  }

  if (add_invert && !m_style[IS_EMPTY])
  {
    append({{ID_EDIT_SELECT_INVERT, _("&Invert")}});
  }

  if (!m_style[IS_READ_ONLY] && m_style[IS_SELECTED] && !m_style[IS_EMPTY])
  {
    append({{wxID_DELETE}});
  }
}

void wex::menu::append_print()
{
  append(
    {{wxID_PRINT_SETUP,
      ellipsed(_("Page &Setup")),
      data::menu().action(
        [=, this](const wxCommandEvent& event)
        {
          wex::printing::get()->get_html_printer()->PageSetup();
        })},
     PRINT_COMPONENT(wxID_PREVIEW, print_preview),
     PRINT_COMPONENT(wxID_PRINT, print)});
}

void wex::menu::append_separator()
{
  if (
    GetMenuItemCount() == 0 ||
    FindItemByPosition(GetMenuItemCount() - 1)->IsSeparator())
  {
    return;
  }

  AppendSeparator();
}
