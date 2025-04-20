////////////////////////////////////////////////////////////////////////////////
// Name:      comands-other.cpp
// Purpose:   Implementation of wex::vi::commands_other
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <charconv>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/ctags/ctags.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/factory/stc-undo.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/vi/vi.h>
#include <wx/app.h>

#include "../util.h"
#include "util.h"

#define REPEAT_WITH_UNDO(TEXT)                                                 \
  {                                                                            \
    stc_undo undo(get_stc());                                                  \
    REPEAT(TEXT);                                                              \
  }

namespace wex
{
size_t fold(wex::syntax::stc* stc, const std::string& command)
{
  if (command.size() <= 1)
  {
    return (size_t)0;
  }

  const auto level = stc->GetFoldLevel(stc->get_current_line());

  const auto line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
                              stc->get_current_line() :
                              stc->GetFoldParent(stc->get_current_line());

  switch (command[1])
  {
    case 'c':
    case 'o':
      if (
        (stc->GetFoldExpanded(line_to_fold) &&
         boost::algorithm::trim_copy(command) == "zc") ||
        (!stc->GetFoldExpanded(line_to_fold) &&
         boost::algorithm::trim_copy(command) == "zo"))
      {
        stc->ToggleFold(line_to_fold);
      }
      break;

    case 'f':
      stc->get_lexer().set_property("fold", "1");
      stc->get_lexer().apply();
      stc->fold(true);
      break;

    case 'E':
      stc->get_lexer().set_property("fold", "0");
      stc->get_lexer().apply();
      stc->fold(false);
      break;

    case 'M':
      stc->fold(true);
      break;

    case 'R':
      for (int i = 0; i < stc->get_line_count(); i++)
      {
        stc->EnsureVisible(i);
      }
      break;
  }

  return command.size();
}

bool replace_char(factory::stc* stc, char c, int count)
{
  if (stc->is_hexmode())
  {
    if (!stc->get_hexmode_replace(c))
    {
      return false;
    }
  }
  else if (stc->GetSelectionMode() == wxSTC_SEL_RECTANGLE)
  {
    for (int i = 0; i < count; i++)
    {
      stc->CharRightRectExtend();
    }

    const auto start = stc->GetSelectionStart();
    const auto end   = stc->GetSelectionEnd();
    stc_undo   undo(stc);

    stc->Cut();

    for (int line = stc->LineFromPosition(start);
         line <= stc->LineFromPosition(end);
         line++)
    {
      stc->InsertText(
        stc->PositionFromLine(line) + stc->GetColumn(start),
        std::string(count, c));
    }
  }
  else
  {
    stc->SetTargetRange(stc->GetCurrentPos(), stc->GetCurrentPos() + count);
    stc->ReplaceTarget(std::string(count, c));
  }

  return true;
}

size_t shift(vi* vi, int count, const std::string& command)
{
  switch (vi->mode().get())
  {
    case vi_mode::state_t::COMMAND:
      command == ">" ? addressrange(vi, count).shift_right() :
                       addressrange(vi, count).shift_left();
      break;

    case vi_mode::state_t::VISUAL:
    case vi_mode::state_t::VISUAL_LINE:
    case vi_mode::state_t::VISUAL_BLOCK:
      command == ">" ?
        addressrange(vi, ex_command::selection_range()).shift_right() :
        addressrange(vi, ex_command::selection_range()).shift_left();
      break;

    default:
      assert(0);
  }

  return 1;
}

size_t word_action(vi* vi, const std::string& command)
{
  const auto start =
    vi->get_stc()->WordStartPosition(vi->get_stc()->GetCurrentPos(), true);
  const auto end =
    vi->get_stc()->WordEndPosition(vi->get_stc()->GetCurrentPos(), true);

  if (const auto word(
        vi->get_stc()->GetSelectedText().empty() ?
          vi->get_stc()->GetTextRange(start, end).ToStdString() :
          vi->get_stc()->GetSelectedText().ToStdString());
      !word.empty())
  {
    if (command == "U")
    {
      if (!browser_search(word))
      {
        return 0;
      }
    }
    else
    {
      find_replace_data::get()->set_find_string(word);
      vi->search_whole_word();
      vi->get_stc()->find(
        find_replace_data::get()->get_find_string(),
        vi->search_flags(),
        command == "*");
    }
  }

  return 1;
}
} // namespace wex

wex::vi::commands_t wex::vi::commands_other()
{
  return {
    {"m",
     [&](const std::string& command)
     {
       if (command.size() == 2)
       {
         marker_add(command.back());
         return 2;
       }
       return 0;
     }},
    {"p",
     [&](const std::string& command)
     {
       REPEAT(put(true));
       return 1;
     }},
    {"q",
     [&](const std::string& command)
     {
       return get_macros().mode().transition(command, this, false, m_count);
     }},
    {"@",
     [&](const std::string& command)
     {
       return get_macros().mode().transition(command, this, false, m_count);
     }},
    {"r",
     [&](const std::string& command)
     {
       if (command.size() > 1 && !get_stc()->GetReadOnly())
       {
         if (!replace_char(get_stc(), command.back(), m_count))
         {
           m_command.clear();
           return 0;
         }
         return 2;
       }
       return 0;
     }},
    {"s",
     [&](const std::string& command)
     {
       const std::string cmd(
         (m_count > 1 ? std::to_string(m_count) : std::string()) + "c ");
       vi::command(cmd);
       return 1;
     }},
    {"u",
     [&](const std::string& command)
     {
       if (get_stc()->CanUndo())
       {
         get_stc()->Undo();
       }
       else
       {
         bell();
       }
       return 1;
     }},
    {k_s(WXK_DELETE) + "x",
     [&](const std::string& command)
     {
       delete_range(
         get_stc()->GetCurrentPos(),
         get_stc()->GetCurrentPos() + m_count);
       return 1;
     }},
    {"J",
     [&](const std::string& command)
     {
       addressrange(this, m_count + 1).join();
       return 1;
     }},
    {"P",
     [&](const std::string& command)
     {
       REPEAT(put(false));
       return 1;
     }},
    {"S",
     [&](const std::string& command)
     {
       const std::string cmd(
         (m_count > 1 ? std::to_string(m_count) : std::string()) + "c_");
       vi::command(cmd);
       return 1;
     }},
    // tag commands ->
    {"Q",
     [&](const std::string& command)
     {
       frame()->save_current_page("ctags");

       if (get_stc()->GetSelectedText().empty())
       {
         get_stc()->GetColumn(get_stc()->GetCurrentPos()) == 0 ?
           ctags::find(std::string()) :
           ctags::find(
             get_stc()->get_word_at_pos(get_stc()->GetCurrentPos()),
             get_stc());
       }
       else
       {
         ctags::find(get_stc()->GetSelectedText().ToStdString(), get_stc());
       }

       return 1;
     }},
    {k_s(WXK_CONTROL_W),
     [&](const std::string& command)
     {
       frame()->restore_page("ctags");
       return 1;
     }},
    {k_s(WXK_CONTROL_T),
     [&](const std::string& command)
     {
       frame()->save_current_page("ctags");
       ctags::previous();
       return 1;
     }},
    {k_s(WXK_CONTROL_V),
     [&](const std::string& command)
     {
       frame()->save_current_page("ctags");
       ctags::next();
       return 1;
     }},
    // <- tag commands
    {"X",
     [&](const std::string& command)
     {
       delete_range(
         get_stc()->GetCurrentPos() - m_count,
         get_stc()->GetCurrentPos());
       return 1;
     }},
    {"dd",
     [&](const std::string& command)
     {
       addressrange(this, m_count).erase();
       return command.size();
     }},
    {"dgg",
     [&](const std::string& command)
     {
       delete_range(
         0,
         get_stc()->PositionFromLine(get_stc()->get_current_line()));
       return 3;
     }},
    {"gg",
     [&](const std::string& command)
     {
       m_mode.is_visual() ? get_stc()->DocumentStartExtend() :
                            get_stc()->DocumentStart();
       return 2;
     }},
    {"yy",
     [&](const std::string& command)
     {
       addressrange(this, m_count).yank();
       return command.size();
     }},
    {"ZZ",
     [&](const std::string& command)
     {
       wxPostEvent(
         wxTheApp->GetTopWindow(),
         wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
       wxPostEvent(wxTheApp->GetTopWindow(), wxCloseEvent(wxEVT_CLOSE_WINDOW));
       return 2;
     }},
    {"z",
     [&](const std::string& command)
     {
       return fold(get_stc(), command);
     }},
    {".",
     [&](const std::string& command)
     {
       if (m_last_command.empty())
       {
         return 1;
       }
       m_dot = true;
       REPEAT_WITH_UNDO(vi::command(m_last_command));
       m_dot = false;
       return 1;
     }},
    {"~",
     [&](const std::string& command)
     {
       return reverse_case(command);
     }},
    {"><",
     [&](const std::string& command)
     {
       return shift(this, m_count, command);
     }},
    {"*#U",
     [&](const std::string& command)
     {
       return word_action(this, command);
     }},
    {"\t",
     [&](const std::string& command)
     {
       // just ignore tab, except on first col, then it indents
       if (get_stc()->GetColumn(get_stc()->GetCurrentPos()) == 0)
       {
         m_command.clear();
         return 0;
       }
       return 1;
     }},
    {k_s(WXK_CONTROL_H),
     [&](const std::string& command)
     {
       if (!get_stc()->GetReadOnly() && !get_stc()->is_hexmode())
       {
         get_stc()->DeleteBack();
       }
       return command.size();
     }},
    {k_s(WXK_CONTROL_J) + k_s(WXK_CONTROL_L),
     [&](const std::string& command)
     {
       return inc_or_dec(command);
     }},
    {k_s(WXK_CONTROL_R),
     [&](const std::string& command)
     {
       if (command.size() > 2)
       {
         command_reg(command);
         return command.size();
       }
       if (is_register_valid(command))
       {
         command_reg(command);
         return command.size();
       }
       return (size_t)0;
     }}};
}

size_t wex::vi::inc_or_dec(const std::string& command)
{
  // clang-format off
  /* NOLINTNEXTLINE */
  REPEAT_WITH_UNDO (
    if (get_stc()->is_hexmode()) return 1;

    try
    {
      const auto start =
        get_stc()->WordStartPosition(get_stc()->GetCurrentPos(), true);
      const auto sign = (get_stc()->GetCharAt(start) == '-' ? 1 : 0);
      const auto end =
        get_stc()->WordEndPosition(get_stc()->GetCurrentPos() + sign, true);
      const std::string word(get_stc()->GetTextRange(start, end).ToStdString());
      int number;
      std::from_chars(word.data(), word.data() + word.size(), number);
      const auto next = (command == k_s(WXK_CONTROL_J) ? ++number : --number);

      std::ostringstream format;
      format.fill(' ');
      format.width(end - start);

      if (word.substr(0, 2) == "0x")
        format << std::hex;
      else if (word[0] == '0')
        format << std::oct;
      format << std::showbase << next;

      get_stc()->wxStyledTextCtrl::Replace(start, end, format.str());
    }
    catch (...)
    {})

    return 1;
  // clang-format on
}

bool wex::vi::other_command(std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  filter_count(command);

  if (const auto& it = std::ranges::find_if(
        m_other_commands,
        [command](auto const& e)
        {
          if (!isalpha(e.first.front()))
          {
            return std::ranges::any_of(
              e.first,
              [command](const auto& p)
              {
                return p == command.front();
              });
          }

          return e.first == command.substr(0, e.first.size());
        });
      it != m_other_commands.end())
  {
    if (const auto parsed = it->second(command); parsed > 0)
    {
      command = command.substr(parsed);
      return true;
    }
  }

  return false;
}

size_t wex::vi::reverse_case(const std::string& command)
{
  if (
    get_stc()->GetLength() == 0 || get_stc()->GetReadOnly() ||
    get_stc()->is_hexmode())
  {
    return 0;
  }

  // clang-format off
  REPEAT_WITH_UNDO (
    if (get_stc()->GetCurrentPos() == get_stc()->GetLength()) return 0;
    auto text(get_stc()->GetTextRange(
      get_stc()->GetCurrentPos(),
      get_stc()->GetCurrentPos() + 1));
    if (text.empty()) return 0;
    islower(text[0]) ? text.UpperCase() : text.LowerCase();
    get_stc()->wxStyledTextCtrl::Replace(
      get_stc()->GetCurrentPos(),
      get_stc()->GetCurrentPos() + 1,
      text);
    get_stc()->CharRight())
    ;
  // clang-format on

  return 1;
}
