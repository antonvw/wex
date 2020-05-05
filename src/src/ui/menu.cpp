////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/lexers.h>
#include <wex/listview.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/printing.h>
#include <wex/stc.h>
#include <wex/tool.h>
#include <wex/util.h> // for wex::ellipsed

wex::menu::menu(menu_t style, const std::vector<menu_item>& items)
  : m_style(style)
{
  append(items);
}

wex::menu::menu(const std::vector<menu_item>& items)
{
  m_style = menu_t().set(DEFAULT);
  append(items);
}

wex::menu::menu(const std::string& title, menu_t style)
  : wxMenu(title)
  , m_style(style)
{
}

size_t wex::menu::append(const std::vector<menu_item>& items)
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
        append({{wxID_EXIT, "", "", "", [=](wxCommandEvent& event) {
                   auto* frame =
                     dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
                   frame->Close(true);
                 }}});
        break;

      case menu_item::PRINT:
        append_print();
        break;

      case menu_item::SEPARATOR:
        append_separator();
        break;

      case menu_item::TOOLS:
        append_tools();
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
  append({{wxID_PRINT_SETUP,
           ellipsed(_("Page &Setup")),
           "",
           "",
           [=](wxCommandEvent& event) {
             wex::printing::get()->get_html_printer()->PageSetup();
           }},
          {wxID_PREVIEW,
           "",
           "",
           "",
           [=](wxCommandEvent& event) {
             auto* frame =
               dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
             if (frame->get_stc() != nullptr)
             {
               frame->get_stc()->print_preview();
             }
             else if (frame->get_listview() != nullptr)
             {
               frame->get_listview()->print_preview();
             }
           }},
          {wxID_PRINT, "", "", "", [=](wxCommandEvent& event) {
             auto* frame =
               dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
             if (frame->get_stc() != nullptr)
             {
               frame->get_stc()->print();
             }
             else if (frame->get_listview() != nullptr)
             {
               frame->get_listview()->print();
             }
           }}});
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

void wex::menu::append_tools()
{
  if (lexers::get()->get_lexers().empty())
  {
    return;
  }

  auto* menuTool = new wex::menu(m_style);

  for (const auto& it : tool().get_tool_info())
  {
    if (!it.second.text().empty())
    {
      menuTool->append({{it.first, it.second.text(), it.second.help_text()}});
    }
  }

  append({{menuTool, _("&Tools"), std::string(), wxID_ANY}});
}
