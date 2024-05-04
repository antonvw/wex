////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind-other.cpp
// Purpose:   Implementation of class wex::stc method bind_other
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/util.h>
#include <wex/stc/auto-complete.h>
#include <wex/stc/bind.h>
#include <wex/stc/stc.h>
#include <wex/ui/debug-entry.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/menu.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent

#include <chrono>

namespace wex
{
void check_double_click(stc* stc, wxKeyEvent& event)
{
  static bool first_click = false;

  // Check whether to generate shift double.
  if (event.GetKeyCode() == WXK_SHIFT)
  {
    static std::chrono::time_point<std::chrono::system_clock> start;

    if (!first_click)
    {
      start       = std::chrono::system_clock::now();
      first_click = true;
    }
    else
    {
      if (const auto milli =
            std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now() - start);
          milli.count() < 500)
      {
        stc->get_frame()->shift_double_click();
      }

      first_click = false;
    }
  }
  else
  {
    first_click = false;
  }
}

void hypertext(stc* stc)
{
  if (const auto match_pos = stc->FindText(
        stc->GetCurrentPos() - 1,
        stc->PositionFromLine(stc->get_current_line()),
        "<");
      match_pos != wxSTC_INVALID_POSITION &&
      stc->GetCharAt(match_pos + 1) != '!')
  {
    if (const auto& match(stc->get_word_at_pos(match_pos + 1));
        match.find('/') != 0 &&
        stc->GetCharAt(stc->GetCurrentPos() - 2) != '/' &&
        (stc->get_lexer().language() == "xml" ||
         stc->get_lexer().is_keyword(match)) &&
        !stc->SelectionIsRectangle())
    {
      if (const std::string add("</" + match + ">"); stc->get_vi().is_active())
      {
        if (
          !stc->get_vi().command(add) ||
          !stc->get_vi().command(std::string(1, WXK_ESCAPE)) ||
          !stc->get_vi().command("%") || !stc->get_vi().command("i"))
        {
          log::status("Autocomplete failed");
        }
      }
      else
      {
        stc->InsertText(stc->GetCurrentPos(), add);
      }
    }
  }
}

menu::menu_t get_style(stc* stc)
{
  menu::menu_t style(menu::menu_t().set(menu::IS_POPUP).set(menu::IS_LINES));

  if (stc->GetReadOnly() || stc->is_hexmode())
  {
    style.set(menu::IS_READ_ONLY);
  }

  if (!stc->GetSelectedText().empty())
  {
    style.set(menu::IS_SELECTED);
  }

  if (stc->GetTextLength() == 0)
  {
    style.set(menu::IS_EMPTY);
  }

  if (stc->get_vi().visual() == ex::mode_t::VISUAL)
  {
    style.set(menu::IS_VISUAL);
  }

  if (stc->CanPaste())
  {
    style.set(menu::CAN_PASTE);
  }

  return style;
}

void margin_menu(stc* stc)
{
  auto* menu = new wex::menu({{id::stc::margin_text_hide, "&Hide"}});

  if (stc->is_visual())
  {
    menu->append({{id::stc::margin_text_blame_revision, "&Blame Revision"}});
    menu->append(
      {{id::stc::margin_text_blame_revision_previous, "&Blame Previous"}});
  }

  menu->append({{}});

  if (auto* author =
        menu->AppendCheckItem(id::stc::margin_text_author, "&Show Author");
      config("blame.author").get(true))
  {
    author->Check();
  }

  if (auto* date =
        menu->AppendCheckItem(id::stc::margin_text_date, "&Show Date");
      config("blame.date").get(true))
  {
    date->Check();
  }

  if (auto* id = menu->AppendCheckItem(id::stc::margin_text_id, "&Show Id");
      config("blame.id").get(true))
  {
    id->Check();
  }

  stc->PopupMenu(menu);

  delete menu;
}
}; // namespace wex

void wex::stc::bind_other()
{
  Bind(
    wxEVT_CHAR,
    [=, this](wxKeyEvent& event)
    {
      key_action(event);
    });

  Bind(
    wxEVT_FIND,
    [=, this](wxFindDialogEvent& event)
    {
      find_next(false);
    });

  Bind(
    wxEVT_FIND_NEXT,
    [=, this](wxFindDialogEvent& event)
    {
      find_next(false);
    });

  Bind(
    wxEVT_FIND_REPLACE,
    [=, this](wxFindDialogEvent& event)
    {
      replace_next(true);
    });

  Bind(
    wxEVT_FIND_REPLACE_ALL,
    [=, this](wxFindDialogEvent& event)
    {
      auto* frd = find_replace_data::get();
      replace_all(frd->get_find_string(), frd->get_replace_string());
    });

  Bind(
    wxEVT_KEY_DOWN,
    [=, this](wxKeyEvent& event)
    {
      if (is_hexmode())
      {
        if (event.GetKeyCode() == WXK_LEFT || event.GetKeyCode() == WXK_RIGHT)
        {
          m_hexmode.set_pos(event);
        }
      }

      if (m_vi->on_key_down(event))
      {
        if (
          event.GetKeyCode() == WXK_RETURN &&
          m_data.flags().test(data::stc::WIN_SINGLE_LINE))
        {
          wxPostEvent(m_data.window().parent(), event);
        }
        else
        {
          event.Skip();
        }
      }

      if (event.GetKeyCode() == WXK_BACK || event.GetKeyCode() == WXK_RETURN)
      {
        m_auto_complete->on_char(event.GetKeyCode());
      }
    });

  Bind(
    wxEVT_KEY_UP,
    [=, this](wxKeyEvent& event)
    {
      event.Skip();
      check_brace();
      check_double_click(this, event);
    });

  Bind(
    wxEVT_LEFT_DOWN,
    [=, this](wxMouseEvent& event)
    {
      event.Skip();
      m_margin_text_click = -1;
    });

  Bind(
    wxEVT_LEFT_DCLICK,
    [=, this](wxMouseEvent& event)
    {
      mouse_action(event);
    });

  Bind(
    wxEVT_LEFT_UP,
    [=, this](wxMouseEvent& event)
    {
      mouse_action(event);
    });

  if (m_data.menu().any())
  {
    Bind(
      wxEVT_RIGHT_UP,
      [=, this](wxMouseEvent& event)
      {
        mouse_action(event);
      });
  }

  bind_set_focus(this);

  Bind(
    wxEVT_STC_AUTOCOMP_COMPLETED,
    [=, this](wxStyledTextEvent& event)
    {
      m_auto_complete->complete(event.GetText().ToStdString());
    });

  Bind(
    wxEVT_STC_CHARADDED,
    [=, this](wxStyledTextEvent& event)
    {
      event.Skip();
      auto_indentation(event.GetKey());
    });

  Bind(
    wxEVT_STC_DO_DROP,
    [=, this](wxStyledTextEvent& event)
    {
      if (is_hexmode() || GetReadOnly())
      {
        event.SetDragResult(wxDragNone);
      }
      event.Skip();
    });

  Bind(
    wxEVT_STC_START_DRAG,
    [=, this](wxStyledTextEvent& event)
    {
      if (is_hexmode() || GetReadOnly())
      {
        event.SetDragAllowMove(false);
      }
      event.Skip();
    });

  Bind(
    wxEVT_STC_DWELLEND,
    [=, this](wxStyledTextEvent& event)
    {
      if (CallTipActive())
      {
        CallTipCancel();
      }
    });

  // if we support automatic fold, this can be removed,
  // not yet possible for wx3.0. And add wxSTC_AUTOMATICFOLD_CLICK
  // to config_dialog, and SetAutomaticFold.
  Bind(
    wxEVT_STC_MARGINCLICK,
    [=, this](wxStyledTextEvent& event)
    {
      margin_action(event);
    });

  Bind(
    wxEVT_STC_MARGIN_RIGHT_CLICK,
    [=, this](wxStyledTextEvent& event)
    {
      if (event.GetMargin() == m_margin_text_number)
      {
        m_margin_text_click = LineFromPosition(event.GetPosition());
        margin_menu(this);
      }
    });

  Bind(
    wxEVT_STC_UPDATEUI,
    [=, this](wxStyledTextEvent& event)
    {
      event.Skip();

      if (event.GetUpdated() & wxSTC_UPDATE_SELECTION)
      {
        m_frame->update_statusbar(this, "PaneInfo");
      }
    });
}

void wex::stc::key_action(wxKeyEvent& event)
{
  if (m_vi->is_active() && m_vi->mode().is_insert())
  {
    m_auto_complete->on_char(event.GetUnicodeKey());
  }

  if (m_vi->visual() == ex::mode_t::OFF)
  {
    event.Skip();
  }
  else if (m_vi->on_char(event))
  {
    if (GetReadOnly() && isalnum(event.GetUnicodeKey()))
    {
      log::status(_("Document is readonly"));
      return;
    }

    if (is_hexmode())
    {
      if (GetOvertype())
      {
        if (m_hexmode.replace(event.GetUnicodeKey()))
        {
          CharRight();
        }
      }
      return;
    }

    if (!m_vi->is_active())
    {
      m_auto_complete->on_char(event.GetUnicodeKey());
    }

    event.Skip();
  }

  if (
    event.GetUnicodeKey() == '>' &&
    get_lexer().scintilla_lexer() == "hypertext")
  {
    hypertext(this);
  }
}

void wex::stc::margin_action(wxStyledTextEvent& event)
{
  m_skip = false;

  if (const auto line = LineFromPosition(event.GetPosition());
      event.GetMargin() == m_margin_folding_number)
  {
    if (const auto level = GetFoldLevel(line);
        (level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
    {
      ToggleFold(line);
    }
    m_margin_text_click = -1;
  }
  else if (event.GetMargin() == m_margin_text_number)
  {
    m_margin_text_click = line;

    if (config("blame.id").get(true))
    {
      m_frame->vcs_annotate_commit(this, line, margin_get_revision_id());
    }
  }
  else if (event.GetMargin() == m_margin_divider_number)
  {
    if (m_frame->debug_is_active())
    {
      m_frame->debug_toggle_breakpoint(line, this);
      m_skip = true;
    }
    else
    {
      event.Skip();
    }
  }
  else
  {
    event.Skip();
  }
}

void wex::stc::mouse_action(wxMouseEvent& event)
{
  try
  {
    if (event.LeftUp())
    {
      properties_message();

      event.Skip();
      check_brace();

      if (
        !m_skip && m_frame->debug_is_active() &&
        matches_one_of(
          path().extension(),
          m_frame->debug_entry()->extensions()))
      {
        const auto& word =
          (!GetSelectedText().empty() ? GetSelectedText().ToStdString() :
                                        get_word_at_pos(GetCurrentPos()));

        if (!word.empty() && isalnum(word[0]))
        {
          m_frame->debug_print(word);
        }
      }

      m_skip = false;
    }
    else if (event.RightUp())
    {
      menu menu(get_style(this));
      build_popup_menu(menu);

      if (menu.GetMenuItemCount() > 0)
      {
        // If last item is a separator, delete it.
        if (wxMenuItem* item =
              menu.FindItemByPosition(menu.GetMenuItemCount() - 1);
            item->IsSeparator())
        {
          menu.Delete(item->GetId());
        }

        PopupMenu(&menu);
      }
    }
    else if (event.LeftDClick())
    {
      m_margin_text_click = -1;

      if (
        GetCurLine().Contains("href") &&
        link_open(link_t().set(LINK_OPEN_MIME)))
      {
        return;
      }

      if (!link_open(link_t().set(LINK_OPEN).set(LINK_OPEN_MIME)))
      {
        event.Skip();
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "mouse action";
    event.Skip();
  }
}
