////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind-other.cpp
// Purpose:   Implementation of class wex::stc method bind_other
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <vector>
#include <wex/config.h>
#include <wex/debug-entry.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/frame.h>
#include <wex/menu.h>
#include <wex/stc-bind.h>
#include <wex/stc.h>
#include <wex/vcs.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent

namespace wex
{
  void hypertext(stc* stc)
  {
    if (const auto match_pos = stc->FindText(
          stc->GetCurrentPos() - 1,
          stc->PositionFromLine(stc->get_current_line()),
          "<");
        match_pos != wxSTC_INVALID_POSITION &&
        stc->GetCharAt(match_pos + 1) != '!')
    {
      if (const auto match(stc->get_word_at_pos(match_pos + 1));
          match.find("/") != 0 &&
          stc->GetCharAt(stc->GetCurrentPos() - 2) != '/' &&
          (stc->get_lexer().language() == "xml" ||
           stc->get_lexer().is_keyword(match)) &&
          !stc->SelectionIsRectangle())
      {
        if (const std::string add("</" + match + ">");
            stc->get_vi().is_active())
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
}; // namespace wex

void wex::stc::bind_other()
{
  Bind(wxEVT_CHAR, [=, this](wxKeyEvent& event) {
    key_action(event);
  });

  Bind(wxEVT_FIND, [=, this](wxFindDialogEvent& event) {
    find_next(false);
  });

  Bind(wxEVT_FIND_NEXT, [=, this](wxFindDialogEvent& event) {
    find_next(false);
  });

  Bind(wxEVT_FIND_REPLACE, [=, this](wxFindDialogEvent& event) {
    replace_next(false);
  });

  Bind(wxEVT_FIND_REPLACE_ALL, [=, this](wxFindDialogEvent& event) {
    auto* frd = find_replace_data::get();
    replace_all(frd->get_find_string(), frd->get_replace_string());
  });

  Bind(wxEVT_KEY_DOWN, [=, this](wxKeyEvent& event) {
    if (is_hexmode())
    {
      if (event.GetKeyCode() == WXK_LEFT || event.GetKeyCode() == WXK_RIGHT)
      {
        m_hexmode.set_pos(event);
      }
    }

    if (m_vi->on_key_down(event))
    {
      event.Skip();
    }

    if (event.GetKeyCode() == WXK_BACK || event.GetKeyCode() == WXK_RETURN)
    {
      m_auto_complete.on_char(event.GetKeyCode());
    }
  });

  Bind(wxEVT_KEY_UP, [=, this](wxKeyEvent& event) {
    event.Skip();
    check_brace();
    m_fold_level = get_fold_level();
  });

  Bind(wxEVT_LEFT_DCLICK, [=, this](wxMouseEvent& event) {
    mouse_action(event);
  });

  Bind(wxEVT_LEFT_UP, [=, this](wxMouseEvent& event) {
    mouse_action(event);
  });

  if (m_data.menu().any())
  {
    Bind(wxEVT_RIGHT_UP, [=, this](wxMouseEvent& event) {
      mouse_action(event);
    });
  }

  Bind(wxEVT_SET_FOCUS, [=, this](wxFocusEvent& event) {
    m_frame->set_find_focus(this);
    event.Skip();
  });

  Bind(wxEVT_STC_AUTOCOMP_COMPLETED, [=, this](wxStyledTextEvent& event) {
    m_auto_complete.activate(event.GetText().ToStdString());
  });

  Bind(wxEVT_STC_CHARADDED, [=, this](wxStyledTextEvent& event) {
    event.Skip();
    auto_indentation(event.GetKey());
  });

  Bind(wxEVT_STC_DO_DROP, [=, this](wxStyledTextEvent& event) {
    if (is_hexmode() || GetReadOnly())
    {
      event.SetDragResult(wxDragNone);
    }
    event.Skip();
  });

  Bind(wxEVT_STC_START_DRAG, [=, this](wxStyledTextEvent& event) {
    if (is_hexmode() || GetReadOnly())
    {
      event.SetDragAllowMove(false);
    }
    event.Skip();
  });

  Bind(wxEVT_STC_DWELLEND, [=, this](wxStyledTextEvent& event) {
    if (CallTipActive())
    {
      CallTipCancel();
    }
  });

  // if we support automatic fold, this can be removed,
  // not yet possible for wx3.0. And add wxSTC_AUTOMATICFOLD_CLICK
  // to config_dialog, and SetAutomaticFold.
  Bind(wxEVT_STC_MARGINCLICK, [=, this](wxStyledTextEvent& event) {
    margin_action(event);
  });

  Bind(wxEVT_STC_MARGIN_RIGHT_CLICK, [=, this](wxStyledTextEvent& event) {
    if (event.GetMargin() == m_margin_text_number)
    {
      auto* menu = new wex::menu({{id::stc::margin_text_hide, "&Hide"}, {}});
      auto* author =
        menu->AppendCheckItem(id::stc::margin_text_author, "&Show Author");
      auto* date =
        menu->AppendCheckItem(id::stc::margin_text_date, "&Show Date");
      auto* id = menu->AppendCheckItem(id::stc::margin_text_id, "&Show Id");

      if (config("blame.author").get(true))
        author->Check();
      if (config("blame.date").get(true))
        date->Check();
      if (config("blame.id").get(true))
        id->Check();

      PopupMenu(menu);
      delete menu;
    }
  });

  Bind(wxEVT_STC_UPDATEUI, [=, this](wxStyledTextEvent& event) {
    event.Skip();

    if (event.GetUpdated() & wxSTC_UPDATE_SELECTION)
    {
      m_frame->update_statusbar(this, "PaneInfo");
    }
  });
}

void wex::stc::key_action(wxKeyEvent& event)
{
  if (!m_vi->is_active())
  {
    if (isalnum(event.GetUnicodeKey()))
    {
      m_adding_chars = true;
    }
  }
  else if (m_vi->mode().is_insert())
  {
    if (isalnum(event.GetUnicodeKey()))
    {
      m_adding_chars = true;
    }

    m_auto_complete.on_char(event.GetUnicodeKey());
  }
  else
  {
    m_adding_chars = false;
  }

  if (m_ex->is_active())
  {
    // prevent skip
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
      m_auto_complete.on_char(event.GetUnicodeKey());
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
      wex::vcs vcs{{get_filename()}};

      if (std::string margin(MarginGetText(line));
          !margin.empty() && vcs.entry().log(get_filename(), get_word(margin)))
      {
        AnnotationSetText(
          line,
          lexer().make_comment(
            boost::algorithm::trim_copy(vcs.entry().get_stdout())));
      }
      else if (!vcs.entry().get_stderr().empty())
      {
        log("margin") << vcs.entry().get_stderr();
      }
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
      m_adding_chars = false;
      m_fold_level   = get_fold_level();

      if (
        !m_skip && m_frame->debug_is_active() &&
        matches_one_of(
          get_filename().extension(),
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
      menu::menu_t style(menu::menu_t().set(menu::IS_POPUP));

      if (GetReadOnly() || is_hexmode())
        style.set(menu::IS_READ_ONLY);
      if (!GetSelectedText().empty())
        style.set(menu::IS_SELECTED);
      if (GetTextLength() == 0)
        style.set(menu::IS_EMPTY);
      if (m_visual)
        style.set(menu::IS_VISUAL);
      if (CanPaste())
        style.set(menu::CAN_PASTE);

      menu menu(style);
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
        return;

      if (!link_open(link_t().set(LINK_OPEN).set(LINK_OPEN_MIME)))
      {
        event.Skip();
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "mouse action";
  }
}
