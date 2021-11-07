////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wex::vi
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/app.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/factory/lexers.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/ctags.h>
#include <wex/vi/macro-mode.h>
#include <wex/vi/macros.h>
#include <wex/vi/vi.h>

#include <sstream>

#include "motion.h"
#include "vim.h"

namespace wex
{
constexpr int c_strcmp(char const* lhs, char const* rhs)
{
  return (('\0' == lhs[0]) && ('\0' == rhs[0])) ? 0 :
         (lhs[0] != rhs[0])                     ? (lhs[0] - rhs[0]) :
                                                  c_strcmp(lhs + 1, rhs + 1);
}

const std::string esc()
{
  return std::string("\x1b");
}

// without this code adding tab in block insert mode fails, it only
// add one tab instead of a line of tabs
bool is_block_insert(vi* vi)
{
  return vi->mode().is_insert() &&
         (vi->get_stc()->SelectionIsRectangle() ||
          vi->get_stc()->GetSelectionMode() == wxSTC_SEL_THIN);
}

const std::string _s(wxKeyCode key)
{
  return std::string(1, key);
}
} // namespace wex

#define MOTION(SCOPE, DIRECTION, COND, WRAP)                                \
  {                                                                         \
    for (auto i = 0; i < m_count; i++)                                      \
    {                                                                       \
      switch (m_mode.get())                                                 \
      {                                                                     \
        case wex::vi_mode::state_t::COMMAND:                                \
        case wex::vi_mode::state_t::INSERT:                                 \
          /* NOLINTNEXTLINE */                                              \
          if (WRAP && c_strcmp((#SCOPE), "Line") == 0)                      \
          {                                                                 \
            if (c_strcmp((#DIRECTION), "Down") == 0)                        \
              get_stc()->LineEnd();                                         \
            else                                                            \
              get_stc()->Home();                                            \
          }                                                                 \
          get_stc()->SCOPE##DIRECTION();                                    \
          break;                                                            \
        case wex::vi_mode::state_t::VISUAL:                                 \
          get_stc()->SCOPE##DIRECTION##Extend();                            \
          break;                                                            \
        case wex::vi_mode::state_t::VISUAL_LINE:                            \
          if (                                                              \
            c_strcmp((#SCOPE), "Char") != 0 &&                              \
            c_strcmp((#SCOPE), "Word") != 0)                                \
            get_stc()->SCOPE##DIRECTION##Extend();                          \
          break;                                                            \
        case wex::vi_mode::state_t::VISUAL_BLOCK:                           \
          get_stc()->SCOPE##DIRECTION##RectExtend();                        \
          break;                                                            \
        default:                                                            \
          break;                                                            \
      }                                                                     \
    }                                                                       \
    /* NOLINTNEXTLINE */                                                    \
    if (c_strcmp((#SCOPE), "Line") == 0)                                    \
    {                                                                       \
      switch (m_mode.get())                                                 \
      {                                                                     \
        case wex::vi_mode::state_t::COMMAND:                                \
        case wex::vi_mode::state_t::INSERT:                                 \
          if (                                                              \
            (COND) &&                                                       \
            get_stc()->GetColumn(get_stc()->GetCurrentPos()) !=             \
              get_stc()->GetLineIndentation(get_stc()->get_current_line())) \
            get_stc()->VCHome();                                            \
          break;                                                            \
        case wex::vi_mode::state_t::VISUAL:                                 \
          if (COND)                                                         \
            get_stc()->VCHomeExtend();                                      \
          break;                                                            \
        default:                                                            \
          break;                                                            \
      }                                                                     \
    }                                                                       \
    return 1;                                                               \
  }

#define REPEAT(TEXT)                   \
  {                                    \
    for (auto i = 0; i < m_count; i++) \
    {                                  \
      TEXT;                            \
    }                                  \
  }

#define REPEAT_WITH_UNDO(TEXT)    \
  {                               \
    get_stc()->BeginUndoAction(); \
    REPEAT(TEXT);                 \
    get_stc()->EndUndoAction();   \
  }

wex::vi::vi(wex::factory::stc* arg, mode_t ex_mode)
  : ex(arg, ex_mode)
  , m_mode(
      this,
      // insert mode process
      [=, this](const std::string& command) {
        if (!m_dot)
        {
          m_insert_text.clear();
        }

        get_stc()->BeginUndoAction();
      },
      // back to command mode process
      [=, this]() {
        if (!m_dot)
        {
          const std::string c(m_insert_command + register_insert());
          set_last_command(c + esc());
          get_macros().record(c);
          get_macros().record(esc(), true);
        }
        m_command.clear();
        m_insert_command.clear();
        get_stc()->EndUndoAction();
      })
  , m_last_commands{{"!", "<", ">", "A", "C", "D", "I", "J", "O",
                     "P", "R", "S", "X", "Y", "a", "c", "d", "g", "i",
                     "o", "p", "r", "s", "x", "y", "~"}}
  , m_motion_commands{{"h",
                       [&](const std::string& command) {
                         if (
                           get_stc()->GetColumn(get_stc()->GetCurrentPos()) > 0)
                           MOTION(Char, Left, false, false);
                         return 1;
                       }},
                      {"j",
                       [&](const std::string& command) {
                         MOTION(Line, Down, false, false);
                       }},
                      {"k",
                       [&](const std::string& command) {
                         MOTION(Line, Up, false, false);
                       }},
                      {"l ",
                       [&](const std::string& command) {
                         if (
                           command == "l" && get_stc()->GetCurrentPos() >=
                                               get_stc()->GetLineEndPosition(
                                                 get_stc()->get_current_line()))
                           return 1;
                         MOTION(Char, Right, false, false);
                       }},
                      {"b",
                       [&](const std::string& command) {
                         MOTION(Word, Left, false, false);
                       }},
                      {"B",
                       [&](const std::string& command) {
                         MOTION(BigWord, Left, false, false);
                       }},
                      {"e",
                       [&](const std::string& command) {
                         MOTION(Word, RightEnd, false, false);
                       }},
                      {"E",
                       [&](const std::string& command) {
                         MOTION(BigWord, RightEnd, false, false);
                       }},
                      {"w",
                       [&](const std::string& command) {
                         MOTION(Word, Right, false, false);
                       }},
                      {"W",
                       [&](const std::string& command) {
                         MOTION(BigWord, Right, false, false);
                       }},
                      {"fFtT,;",
                       [&](const std::string& command) {
                         if (command.empty())
                           return (size_t)0;
                         char c; // char to find
                         if (command.size() == 1)
                         {
                           if (command[0] == ';' || command[0] == ',')
                           {
                             if (m_last_find_char_command.empty())
                               return (size_t)0;
                             c = m_last_find_char_command.back();
                           }
                           else
                           {
                             return (size_t)0;
                           }
                         }
                         else
                         {
                           c = command[1];
                         }
                         char d; // char specifying direction
                         switch (command[0])
                         {
                           case ';':
                             d = m_last_find_char_command.front();
                             break;
                           case ',':
                             d = m_last_find_char_command.front();
                             if (islower(d))
                               d = toupper(d);
                             else
                               d = tolower(d);
                             break;
                           default:
                             if (command.size() > 1)
                               d = command.front();
                             else
                               d = m_last_find_char_command.front();
                         }
                         REPEAT(if (!get_stc()->find(
                                      std::string(1, c),
                                      0,
                                      islower(d) > 0)) {
                           m_command.clear();
                           return (size_t)0;
                         });
                         if (command[0] != ',' && command[0] != ';')
                         {
                           m_last_find_char_command = command;
                         }
                         if (tolower(d) == 't')
                         {
                           get_stc()->CharLeft();
                         }
                         return command.size();
                       }},
                      {"nN",
                       [&](const std::string& command) {
                         REPEAT(
                           if (const std::string find(
                                 get_stc()->get_margin_text_click() > 0 ?
                                   config("ex-cmd.margin")
                                     .get(config::strings_t{})
                                     .front() :
                                   find_replace_data::get()->get_find_string());
                               !get_stc()->find(
                                 find,
                                 search_flags(),
                                 command == "n" == m_search_forward)) {
                             m_command.clear();
                             return (size_t)0;
                           });
                         return (size_t)1;
                       }},
                      {"G",
                       [&](const std::string& command) {
                         if (m_count == 1 && !m_count_present)
                         {
                           m_mode.is_visual() ? get_stc()->DocumentEndExtend() :
                                             get_stc()->DocumentEnd();
                         }
                         else
                         {
                           get_stc()->inject(data::control().line(m_count));
                         }
                         return 1;
                       }},
                      {"H",
                       [&](const std::string& command) {
                         get_stc()->goto_line(get_stc()->GetFirstVisibleLine());
                         return 1;
                       }},
                      {"L",
                       [&](const std::string& command) {
                         get_stc()->goto_line(
                           get_stc()->GetFirstVisibleLine() +
                           get_stc()->LinesOnScreen() - 1);
                         return 1;
                       }},
                      {"M",
                       [&](const std::string& command) {
                         get_stc()->goto_line(
                           get_stc()->GetFirstVisibleLine() +
                           get_stc()->LinesOnScreen() / 2);
                         return 1;
                       }},
                      {"/?",
                       [&](const std::string& command) {
                         m_search_forward = command.front() == '/';

                         if (command.size() > 1)
                         {
                           if (command[1] == WXK_CONTROL_R)
                           {
                             if (command.size() < 3)
                             {
                               return (size_t)0;
                             }

                             if (!get_stc()->find(
                                   get_macros().get_register(command[2]),
                                   search_flags(),
                                   m_search_forward))
                               return (size_t)0;
                             find_replace_data::get()->set_find_string(
                               get_macros().get_register(command[2]));
                             return command.size();
                           }

                           // This is a previous entered command.
                           // The trim is to enable find after -E option in
                           // syncped.
                           const auto text(
                             command.back() != '\n' ? command.substr(1) :
                                                      boost::algorithm::trim_copy(command.substr(1)));

                           if (!get_stc()->find(
                                 text,
                                 search_flags(),
                                 m_search_forward))
                             return (size_t)0;
                           if (get_stc()->get_margin_text_click() == -1)
                             find_replace_data::get()->set_find_string(text);
                           return command.size();
                         }
                         else
                         {
                           return get_stc()->is_visual() && frame()->show_ex_command(
                                    get_stc(),
                                    command +
                                      (m_mode.is_visual() ? "'<,'>" : "")) ?
                                    command.size() :
                                    (size_t)0;
                         }
                       }},
                      {"\'",
                       [&](const std::string& command) {
                         if (one_letter_after("'", command))
                         {
                           const auto pos = get_stc()->GetCurrentPos();
                           marker_goto(command.back());
                           visual_extend(pos, get_stc()->GetCurrentPos());
                           return 2;
                         }
                         return 0;
                       }},
                      {"0",
                       [&](const std::string& command) {
                         MOTION(Line, Home, false, false);
                       }},
                      {"^",
                       [&](const std::string& command) {
                         MOTION(VC, Home, false, false);
                       }},
                      {"[]",
                       [&](const std::string& command) {
                         // related to regex ECMAScript
                         REPEAT(if (!get_stc()->find(
                                      "\\{",
                                      search_flags(),
                                      command == "]")) {
                           m_command.clear();
                           return 0;
                         })
                         return 1;
                       }},
                      {"({",
                       [&](const std::string& command) {
                         MOTION(Para, Up, false, false);
                       }},
                      {")}",
                       [&](const std::string& command) {
                         MOTION(Para, Down, false, false);
                       }},
                      {"+",
                       [&](const std::string& command) {
                         MOTION(Line, Down, true, true);
                       }},
                      {"|",
                       [&](const std::string& command) {
                         get_stc()->GotoPos(
                           get_stc()->PositionFromLine(
                             get_stc()->get_current_line()) +
                           m_count - 1);
                         return 1;
                       }},
                      {"-",
                       [&](const std::string& command) {
                         MOTION(Line, Up, true, true);
                       }},
                      {"$",
                       [&](const std::string& command) {
                         MOTION(Line, End, false, false);
                       }},
                      {"%",
                       [&](const std::string& command) {
                         auto pos         = get_stc()->GetCurrentPos();
                         auto brace_match = get_stc()->BraceMatch(pos);

                         if (brace_match == wxSTC_INVALID_POSITION)
                         {
                           brace_match = get_stc()->BraceMatch(--pos);
                         }

                         if (brace_match != wxSTC_INVALID_POSITION)
                         {
                           get_stc()->GotoPos(brace_match);
                           visual_extend(pos, brace_match + 1);
                         }
                         else
                         {
                           get_stc()->find(
                             "[(\\[{\\])}]",
                             wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX);
                         }
                         return 1;
                       }},
                      {"!",
                       [&](const std::string& command) {
                         if (command.size() > 1)
                         {
                           ex::command(":" + command);
                           return command.size();
                         }
                         else if (get_stc()->is_visual())
                         {
                           frame()->show_ex_command(get_stc(), command);
                           return (size_t)1;
                         }
                         else
                         {
                           return (size_t)1;
                         }
                       }},
                      {"\r_",
                       [&](const std::string& command) {
                         get_stc()->Home();
                         if (command.front() == '_')
                           m_count--;
                         MOTION(Line, Down, false, false);
                       }},
                      {_s(WXK_CONTROL_B),
                       [&](const std::string&) {
                         MOTION(Page, Up, false, false);
                       }},
                      {_s(WXK_CONTROL_D),
                       [&](const std::string&) {
                         MOTION(Page, ScrollDown, false, false);
                       }},
                      {_s(WXK_CONTROL_E),
                       [&](const std::string&) {
                         MOTION(Line, ScrollDown, false, false);
                       }},
                      {_s(WXK_CONTROL_F),
                       [&](const std::string&) {
                         MOTION(Page, Down, false, false);
                       }},
                      {_s(WXK_CONTROL_M),
                       [&](const std::string&) {
                         MOTION(Line, Down, false, false);
                       }},
                      {_s(WXK_CONTROL_P),
                       [&](const std::string&) {
                         MOTION(Line, Up, false, false);
                       }},
                      {_s(WXK_CONTROL_U),
                       [&](const std::string&) {
                         MOTION(Page, ScrollUp, false, false);
                       }},
                      {_s(WXK_CONTROL_Y),
                       [&](const std::string&) {
                         MOTION(Line, ScrollUp, false, false);
                       }}}
  , m_other_commands{
      {"m",
       [&](const std::string& command) {
         if (one_letter_after("m", command))
         {
           marker_add(command.back());
           return 2;
         }
         return 0;
       }},
      {"p",
       [&](const std::string& command) {
         REPEAT(put(true));
         return 1;
       }},
      {"q",
       [&](const std::string& command) {
         return get_macros().mode().transition(command, this, false, m_count);
       }},
      {"@",
       [&](const std::string& command) {
         return get_macros().mode().transition(command, this, false, m_count);
       }},
      {"r",
       [&](const std::string& command) {
         if (command.size() > 1 && !get_stc()->GetReadOnly())
         {
           if (get_stc()->is_hexmode())
           {
             if (!get_stc()->get_hexmode_replace(command.back()))
             {
               m_command.clear();
               return 0;
             }
           }
           else
           {
             get_stc()->SetTargetRange(
               get_stc()->GetCurrentPos(),
               get_stc()->GetCurrentPos() + m_count);
             get_stc()->ReplaceTarget(std::string(m_count, command.back()));
           }
           return 2;
         }
         return 0;
       }},
      {"s",
       [&](const std::string& command) {
         const std::string cmd((m_count > 1 ? 
           std::to_string(m_count): std::string()) + "c ");
         vi::command(cmd);
         return 1;
       }},
      {"u",
       [&](const std::string& command) {
         if (get_stc()->CanUndo())
           get_stc()->Undo();
         else
         {
           if (config(_("Error bells")).get(true))
           {
             wxBell();
           }
         }
         return 1;
       }},
      {"x",
       [&](const std::string& command) {
         delete_range(
           get_stc()->GetCurrentPos(),
           get_stc()->GetCurrentPos() + m_count);
         return 1;
       }},
      {"J",
       [&](const std::string& command) {
         addressrange(this, m_count + 1).join();
         return 1;
       }},
      {"P",
       [&](const std::string& command) {
         REPEAT(put(false));
         return 1;
       }},
      {"S",
       [&](const std::string& command) {
         const std::string cmd((m_count > 1 ? 
           std::to_string(m_count): std::string()) + "c_");
         vi::command(cmd);
         return 1;
       }},
      // tag commands ->
      {"Q",
       [&](const std::string& command) {
         frame()->save_current_page("ctags");

         if (get_stc()->GetSelectedText().empty())
         {
           get_stc()->GetColumn(get_stc()->GetCurrentPos()) == 0 ?
             ctags::find(std::string()) :
             ctags::find(
               get_stc()->get_word_at_pos(get_stc()->GetCurrentPos()),
               this);
         }
         else
         {
           ctags::find(get_stc()->GetSelectedText().ToStdString(), this);
         }

         return 1;
       }},
      {_s(WXK_CONTROL_W),
       [&](const std::string& command) {
         frame()->restore_page("ctags");
         return 1;
       }},
      {_s(WXK_CONTROL_T),
       [&](const std::string& command) {
         frame()->save_current_page("ctags");
         ctags::previous();
         return 1;
       }},
      {_s(WXK_CONTROL_V),
       [&](const std::string& command) {
         frame()->save_current_page("ctags");
         ctags::next();
         return 1;
       }},
      // <- tag commands
      {"X",
       [&](const std::string& command) {
         delete_range(
           get_stc()->GetCurrentPos() - m_count,
           get_stc()->GetCurrentPos());
         return 1;
       }},
      {"dd",
       [&](const std::string& command) {
         addressrange(this, m_count).erase();
         return command.size();
       }},
      {"dgg",
       [&](const std::string& command) {
         delete_range(
           0,
           get_stc()->PositionFromLine(get_stc()->get_current_line()));
         return 3;
       }},
      {"gg",
       [&](const std::string& command) {
         m_mode.is_visual() ? get_stc()->DocumentStartExtend() :
                           get_stc()->DocumentStart();
         return 2;
       }},
      {"yy",
       [&](const std::string& command) {
         addressrange(this, m_count).yank();
         return command.size();
       }},
      {"ZZ",
       [&](const std::string& command) {
         wxPostEvent(
           wxTheApp->GetTopWindow(),
           wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
         wxPostEvent(
           wxTheApp->GetTopWindow(),
           wxCloseEvent(wxEVT_CLOSE_WINDOW));
         return 2;
       }},
      {"z",
       [&](const std::string& command) {
         if (command.size() <= 1)
           return (size_t)0;
         const auto level =
           get_stc()->GetFoldLevel(get_stc()->get_current_line());
         const auto line_to_fold =
           (level & wxSTC_FOLDLEVELHEADERFLAG) ?
             get_stc()->get_current_line() :
             get_stc()->GetFoldParent(get_stc()->get_current_line());

         switch (command[1])
         {
           case 'c':
           case 'o':
             if (
               get_stc()->GetFoldExpanded(line_to_fold) &&
               boost::algorithm::trim_copy(command) == "zc")
               get_stc()->ToggleFold(line_to_fold);
             else if (
               !get_stc()->GetFoldExpanded(line_to_fold) &&
               boost::algorithm::trim_copy(command) == "zo")
               get_stc()->ToggleFold(line_to_fold);
             break;
           case 'f':
             get_stc()->get_lexer().set_property("fold", "1");
             get_stc()->get_lexer().apply();
             get_stc()->fold(true);
             break;
           case 'E':
             get_stc()->get_lexer().set_property("fold", "0");
             get_stc()->get_lexer().apply();
             get_stc()->fold(false);
             break;
           case 'M':
             get_stc()->fold(true);
             break;
           case 'R':
             for (int i = 0; i < get_stc()->get_line_count(); i++)
               get_stc()->EnsureVisible(i);
             break;
         };
         return command.size();
       }},
      {".",
       [&](const std::string& command) {
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
       [&](const std::string& command) {
         if (
           get_stc()->GetLength() == 0 || get_stc()->GetReadOnly() ||
           get_stc()->is_hexmode())
           return 0;
         REPEAT_WITH_UNDO(
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
           get_stc()->CharRight());
         return 1;
       }},
      {"><",
       [&](const std::string& command) {
         switch (m_mode.get())
         {
           case vi_mode::state_t::COMMAND:
             command == ">" ? addressrange(this, m_count).shift_right() :
                              addressrange(this, m_count).shift_left();
             break;
           case vi_mode::state_t::VISUAL:
           case vi_mode::state_t::VISUAL_LINE:
           case vi_mode::state_t::VISUAL_BLOCK:
             command == ">" ? addressrange(this, "'<,'>").shift_right() :
                              addressrange(this, "'<,'>").shift_left();
             break;
           default:
             break;
         }
         return 1;
       }},
      {"*#U",
       [&](const std::string& command) {
         const auto start =
           get_stc()->WordStartPosition(get_stc()->GetCurrentPos(), true);
         const auto end =
           get_stc()->WordEndPosition(get_stc()->GetCurrentPos(), true);

         if (const std::string word(
               get_stc()->GetSelectedText().empty() ?
                 get_stc()->GetTextRange(start, end).ToStdString() :
                 get_stc()->GetSelectedText().ToStdString());
             !word.empty())
         {
           if (command == "U")
           {
             browser_search(word);
           }
           else
           {
             find_replace_data::get()->set_find_string(word);
             get_stc()->find(
               find_replace_data::get()->get_find_string(),
               search_flags() | wxSTC_FIND_WHOLEWORD,
               command == "*");
           }
         }
         return 1;
       }},
      {"\t",
       [&](const std::string& command) {
         // just ignore tab, except on first col, then it indents
         if (get_stc()->GetColumn(get_stc()->GetCurrentPos()) == 0)
         {
           m_command.clear();
           return 0;
         }
         return 1;
       }},
      {_s(WXK_CONTROL_H),
       [&](const std::string& command) {
         if (!get_stc()->GetReadOnly() && !get_stc()->is_hexmode())
           get_stc()->DeleteBack();
         return command.size();
       }},
      {_s(WXK_CONTROL_J) + _s(WXK_CONTROL_L),
       [&](const std::string& command) {
         /* NOLINTNEXTLINE */
         REPEAT_WITH_UNDO(if (get_stc()->is_hexmode()) return 1; try {
           const auto start =
             get_stc()->WordStartPosition(get_stc()->GetCurrentPos(), true);
           const auto sign = (get_stc()->GetCharAt(start) == '-' ? 1 : 0);
           const auto end  = get_stc()->WordEndPosition(
             get_stc()->GetCurrentPos() + sign,
             true);
           const std::string word(
             get_stc()->GetTextRange(start, end).ToStdString());
           auto       number = std::stoi(word, nullptr, 0);
           const auto next =
             (command == _s(WXK_CONTROL_J) ? ++number : --number);
           std::ostringstream format;
           format.fill(' ');
           format.width(end - start);
           if (word.substr(0, 2) == "0x")
             format << std::hex;
           else if (word[0] == '0')
             format << std::oct;
           format << std::showbase << next;
           get_stc()->wxStyledTextCtrl::Replace(start, end, format.str());
         } catch (...){})
         return 1;
       }},
      {_s(WXK_CONTROL_R),
       [&](const std::string& command) {
         if (command.size() > 2)
         {
           command_reg(command);
           return command.size();
         }
         else if (command.size() == 2 && regafter(_s(WXK_CONTROL_R), command))
         {
           command_reg(command);
           return command.size();
         }
         return (size_t)0;
       }},
      // delete char
      {"\x7F", [&](const std::string& command) {
         delete_range(
           get_stc()->GetCurrentPos(),
           get_stc()->GetCurrentPos() + m_count);
         return 1;
       }}}
{
}

void wex::vi::append_insert_command(const std::string& s)
{
  m_insert_command.append(s);
  log::trace("insert command") << m_insert_command;
}

void wex::vi::append_insert_text(const std::string& s)
{
  m_insert_text.append(s);
  set_register_insert(m_insert_text);
  log::trace("insert text") << m_insert_text;
}

bool wex::vi::command(const std::string& command)
{
  if (command.empty() || !is_active())
  {
    return false;
  }

  if (command.front() != ':' && command.front() != '!')
  {
    log::trace("vi command") << command;
  }

  if (
    m_mode.is_visual() && command.find("'<,'>") == std::string::npos &&
    ex::command(command + "'<,'>"))
  {
    return auto_write();
  }
  else if (m_mode.is_insert())
  {
    return insert_mode(command);
  }
  else if (
    !m_dot && command.size() > 2 && command.back() == WXK_ESCAPE &&
    command[command.size() - 2] == WXK_ESCAPE)
  {
    m_mode.escape();
    m_command.clear();
    return auto_write();
  }

  if (auto parse(command); !parse_command(parse))
  {
    return false;
  }
  else
  {
    if (!m_dot)
    {
      set_last_command(command);
    }

    if (
      !m_mode.is_insert() && command[0] != 'q' && command[0] != ':' &&
      command[0] != '!' && command != "/" && command != "?" &&
      command != _s(WXK_CONTROL_R) + "=")
    {
      get_macros().record(command);
    }

    return auto_write();
  }
}

void wex::vi::command_reg(const std::string& reg)
{
  switch (reg[0])
  {
    case 0:
      break;

    // calc register: control-R =
    case WXK_CONTROL_R:
      if (reg.size() > 1 && reg[1] == '=' && get_stc()->is_visual())
      {
        if (reg.size() == 2)
        {
          set_register_insert(std::string());
          frame()->show_ex_command(get_stc(), reg);
        }
        else
        {
          const auto sum = calculator(reg.substr(2));

          if (m_mode.is_insert())
          {
            if (m_last_command.find('c') != std::string::npos)
            {
              get_stc()->ReplaceSelection(wxEmptyString);
            }

            get_stc()->add_text(std::to_string(sum));

            append_insert_command(reg);
          }
          else
          {
            set_register_yank(std::to_string(sum));
            frame()->show_ex_message(std::to_string(sum));
          }
        }
      }
      else
      {
        frame()->show_ex_message("calc register is control-R =");
      }
      break;

    // clipboard register
    case '*':
      if (m_mode.is_insert())
      {
        put(true);
      }
      break;

    // filename register
    case '%':
      if (m_mode.is_insert())
      {
        get_stc()->add_text(get_stc()->path().filename());
      }
      else
      {
        frame()->show_ex_message(get_stc()->path().string());
        clipboard_add(get_stc()->path().string());
      }
      break;

    default:
      if (!get_macros().get_register(reg[0]).empty())
      {
        if (m_mode.is_insert())
        {
          get_stc()->add_text(get_macros().get_register(reg[0]));

          if (reg[0] == '.')
          {
            append_insert_text(register_insert());
          }
        }
        else
        {
          frame()->show_ex_message(get_macros().get_register(reg[0]));
        }
      }
      else
      {
        frame()->show_ex_message("?" + reg);
      }
  }
}

char wex::vi::convert_key_event(const wxKeyEvent& event) const
{
  if (event.GetKeyCode() == WXK_BACK)
    return WXK_BACK;
  else if (event.GetKeyCode() == WXK_RETURN && !m_mode.is_insert())
    return 'j';
  else if (event.GetModifiers() & wxMOD_RAW_CONTROL)
    return event.GetKeyCode();

  char c = event.GetUnicodeKey();

  if (c == WXK_NONE)
  {
    switch (event.GetKeyCode())
    {
      case WXK_LEFT:
        c = 'h';
        break;
      case WXK_DOWN:
        c = 'j';
        break;
      case WXK_UP:
        c = 'k';
        break;
      case WXK_RIGHT:
        c = 'l';
        break;
      case WXK_DELETE:
        c = 'x';
        break;
      case WXK_PAGEUP:
        c = WXK_CONTROL_B;
        break;
      case WXK_PAGEDOWN:
        c = WXK_CONTROL_F;
        break;
      case WXK_NUMPAD_ENTER:
        c = 'j';
        break;
      default:
        c = event.GetKeyCode();
    }
  }

  return c;
}

bool wex::vi::delete_range(int start, int end)
{
  if (get_stc()->GetReadOnly())
  {
    return false;
  }

  const auto first = (start < end ? start : end);
  const auto last  = (start < end ? end : start);

  if (!get_stc()->is_hexmode())
  {
    const auto& b(get_stc()->GetTextRangeRaw(first, last));

    get_macros().set_register(
      register_name() ? register_name() : '0',
      std::string(b.data(), b.length()));

    get_stc()->DeleteRange(first, last - first);
  }
  else
  {
    get_stc()->get_hexmode_erase(last - first, first);
  }

  return true;
}

void wex::vi::filter_count(std::string& command)
{
  /*
   command: 3w
   -> v has 2 elements
   -> m_count 3
   -> command w
   */
  if (regex v("^([1-9][0-9]*)(.*)"); v.match(command) == 2)
  {
    try
    {
      m_count_present  = true;
      const auto count = std::stoi(v[0]);
      m_count *= count;
      append_insert_command(v[0]);
      command = v[1];
    }
    catch (std::exception& e)
    {
      m_count_present = false;
      log(e) << command;
      command = v[1];
    }
  }
}

wex::vi::motion_t wex::vi::get_motion(const std::string& command) const
{
  switch (command[0])
  {
    case 'c':
      return motion_t::CHANGE;

    case 'd':
      return motion_t::DEL;

    case 'g':
      return command_g_motion(command);

    case 'y':
      return motion_t::YANK;

    default:
      return motion_t::NAVIGATE;
  }
}

bool wex::vi::insert_mode(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  else if (get_stc()->is_hexmode())
  {
    if (static_cast<int>(command.back()) == WXK_ESCAPE)
    {
      if (m_mode.escape())
      {
        get_stc()->SetOvertype(false);
      }

      return true;
    }
    else
    {
      return get_stc()->get_hexmode_insert(command, get_stc()->GetCurrentPos());
    }
  }
  // add control chars
  else if (command.size() == 2 && command[1] == 0)
  {
    append_insert_text(std::string(1, command[0]));
    get_stc()->add_text(std::string(1, command[0]));
    return true;
  }

  if (command.starts_with(_s(WXK_CONTROL_R) + "="))
  {
    command_reg(command);
    return true;
  }
  else if (command.find(_s(WXK_CONTROL_R)) != std::string::npos)
  {
    if (command.size() < 2)
    {
      return false;
    }

    std::string text(command);
    marker_and_register_expansion(this, text);
    insert_mode(text);
    return true;
  }

  switch (command.back())
  {
    case WXK_BACK:
      if (!m_insert_text.empty())
      {
        m_insert_text.pop_back();
      }
      get_stc()->DeleteBack();
      break;

    case WXK_CONTROL_R:
      append_insert_text(command);
      break;

    case WXK_DELETE:
      delete_range(get_stc()->GetCurrentPos(), get_stc()->GetCurrentPos() + 1);
      break;

    case WXK_ESCAPE:
      // Add extra inserts if necessary.
      if (!m_insert_text.empty())
      {
        for (auto i = 1; i < m_count; i++)
        {
          insert_mode_normal(m_insert_text);
        }

        set_register_insert(m_insert_text);
      }

      // If we have text to be added.
      if (command.size() > 1)
      {
        if (const auto rest(command.substr(0, command.size() - 1));
            !get_stc()->GetSelectedText().empty())
        {
          get_stc()->ReplaceSelection(rest);
        }
        else
        {
          if (!get_stc()->GetOvertype())
          {
            REPEAT(get_stc()->add_text(rest));
          }
          else
          {
            std::string text;
            get_stc()->SetTargetStart(get_stc()->GetCurrentPos());
            REPEAT(text += rest;);
            get_stc()->SetTargetEnd(get_stc()->GetCurrentPos() + text.size());
            get_stc()->ReplaceTarget(text);
          }
        }
      }

      if (m_mode.escape())
      {
        get_stc()->SetOvertype(false);
      }
      break;

    default:
      if (
        m_last_command.find('c') != std::string::npos && m_insert_text.empty())
      {
        get_stc()->ReplaceSelection(wxEmptyString);
      }

      if (!m_insert_text.empty() && m_insert_text.back() == WXK_CONTROL_R)
      {
        get_stc()->ReplaceSelection(wxEmptyString);

        if (command.back() != '.')
        {
          append_insert_text(command);
        }
        else
        {
          m_insert_text.pop_back();
        }

        command_reg(std::string(1, command.back()));
        return false;
      }
      else
      {
        if (
          command.size() == 1 &&
          (static_cast<int>(command.back()) == WXK_RETURN ||
           static_cast<int>(command.back()) == WXK_NUMPAD_ENTER))
        {
          get_stc()->NewLine();

          if (!get_stc()->AutoCompActive())
          {
            append_insert_text(get_stc()->eol());
          }
        }
        else
        {
          if (!get_stc()->GetOvertype())
          {
            insert_mode_normal(command);
          }

          if (!m_dot)
          {
            append_insert_text(command);
          }
        }
      }
  }

  return true;
}

void wex::vi::insert_mode_normal(const std::string& text)
{
  if (boost::tokenizer<boost::char_separator<char>> tok(
        text,
        boost::char_separator<char>("", "\r\n", boost::keep_empty_tokens));
      text.find('\0') == std::string::npos &&
      std::distance(tok.begin(), tok.end()) >= 1)
  {
    for (auto it = tok.begin(); it != tok.end(); ++it)
    {
      if (auto token(*it); !token.empty())
      {
        if (token.back() == ' ' || token.back() == '\t' || token.back() == ';')
        {
          if (!m_insert_text.empty())
          {
            const auto last(m_insert_text.find_last_of(" ;\t"));

            if (const auto& it = get_macros().get_abbreviations().find(
                  m_insert_text.substr(last + 1));
                it != get_macros().get_abbreviations().end())
            {
              m_insert_text.replace(last + 1, it->first.size(), it->second);

              const auto pos       = get_stc()->GetCurrentPos();
              const auto match_pos = get_stc()->FindText(
                pos,
                get_stc()->PositionFromLine(get_stc()->get_current_line()),
                it->first);

              if (match_pos != wxSTC_INVALID_POSITION)
              {
                get_stc()->SetTargetRange(
                  match_pos,
                  match_pos + it->first.size());
                get_stc()->ReplaceTarget(it->second);
                get_stc()->SetCurrentPos(
                  pos + it->second.size() - it->first.size());
              }
            }
          }
          else
          {
            const auto last(token.find_last_of(" ;\t", token.size() - 2));
            const auto word(token.substr(last + 1, token.size() - 2 - last));

            if (const auto& it = get_macros().get_abbreviations().find(word);
                it != get_macros().get_abbreviations().end())
            {
              token.replace(last + 1, it->first.size(), it->second);
            }
          }
        }

        get_stc()->add_text(token);

        if (token == "\n")
        {
          get_stc()->auto_indentation(token[0]);
        }
      }
    }
  }
  else
  {
    get_stc()->add_text(text);
  }
}

bool wex::vi::motion_command(motion_t type, std::string& command)
{
  if (!get_stc()->get_selected_text().empty() && command.size() <= 1)
  {
    if (type == motion_t::YANK)
    {
      return addressrange(this, m_count).yank();
    }
    else if (type == motion_t::DEL || type == motion_t::CHANGE)
    {
      return addressrange(this, m_count).erase();
    }
  }

  if (command.empty())
  {
    return false;
  }

  filter_count(command);

  const auto& it = std::find_if(
    m_motion_commands.begin(),
    m_motion_commands.end(),
    [&](auto const& e)
    {
      return std::any_of(
        e.first.begin(),
        e.first.end(),
        [command](const auto& p)
        {
          return p == command[0];
        });
    });

  if (it == m_motion_commands.end())
  {
    return false;
  }

  if (type < motion_t::NAVIGATE && get_stc()->GetReadOnly())
  {
    command.clear();
    return true;
  }

  int  parsed = 0;
  auto start  = get_stc()->GetCurrentPos();

  if (type > motion_t::G_aa && type < motion_t::G_ZZ)
  {
    if ((parsed = it->second(command)) == 0)
      return false;

    command_g(this, type, start);
  }
  else
  {
    switch (type)
    {
      case motion_t::CHANGE:
        if (!get_stc()->get_selected_text().empty())
        {
          get_stc()->SetCurrentPos(get_stc()->GetSelectionStart());
          start = get_stc()->GetCurrentPos();
        }

        if ((parsed = it->second(command)) == 0)
        {
          m_mode.escape();
          return false;
        }

        delete_range(start, get_stc()->GetCurrentPos());
        break;

      case motion_t::DEL:
        if ((parsed = it->second(command)) == 0)
          return false;

        delete_range(start, get_stc()->GetCurrentPos());
        break;

      case motion_t::G:
        return false;
        break;

      case motion_t::NAVIGATE:
        if ((parsed = it->second(command)) == 0)
          return false;
        break;

      case motion_t::YANK:
        if (!m_mode.is_visual())
        {
          std::string visual("v");
          m_mode.transition(visual);
        }

        if ((parsed = it->second(command)) == 0)
        {
          return false;
        }

        yank_range(start);
        break;

      default:
        assert(0);
    }
  }

  if (!m_insert_command.empty())
  {
    append_insert_command(command.substr(0, parsed));
  }

  command = command.substr(parsed);

  m_count         = 1; // restart with a new count
  m_count_present = false;

  return true;
}

bool wex::vi::on_char(const wxKeyEvent& event)
{
  if (!is_active())
  {
    return true;
  }
  else if (m_mode.is_insert())
  {
    if (is_block_insert(this))
    {
      return true;
    }

    m_command.append(convert_key_event(event));
    const bool result = insert_mode(m_command.command());

    if (result || (get_stc()->is_hexmode() && m_command.size() > 2))
    {
      m_command.clear();
    }

    return result && get_stc()->GetOvertype();
  }
  else
  {
    if (!(event.GetModifiers() & wxMOD_ALT))
    {
      // This check is important, as WXK_NONE (0)
      // would add nullptr terminator at the end of m_command,
      // and pressing ESC would not help, (rest is empty
      // because of the nullptr).
      if (event.GetUnicodeKey() != (wxChar)WXK_NONE)
      {
        if (
          !m_command.empty() && m_command.front() == '@' &&
          event.GetKeyCode() == WXK_BACK)
        {
          m_command.pop_back();
        }
        else
        {
#ifdef __WXOSX__
          if (event.GetModifiers() & wxMOD_RAW_CONTROL)
          {
            if (m_command.append_exec(event.GetKeyCode()))
            {
              m_command.clear();
            }
          }
          else
#endif
            if (m_command.append_exec(event.GetUnicodeKey()))
          {
            m_command.clear();
          }
        }
      }
      else
      {
        return true;
      }

      return false;
    }
    else
    {
      return true;
    }
  }
}

bool wex::vi::on_key_down(const wxKeyEvent& event)
{
  if (!is_active() || get_stc()->AutoCompActive())
  {
    return true;
  }
  else if (!m_command.empty() && m_command.front() == '@')
  {
    if (event.GetKeyCode() == WXK_BACK)
    {
      m_command.pop_back();
      frame()->statustext(m_command.command().substr(1), "PaneMacro");
    }
    else if (event.GetKeyCode() == WXK_ESCAPE)
    {
      m_command.clear();
      m_insert_command.clear();
      frame()->statustext(get_macros().mode().get_macro(), "PaneMacro");
    }

    return true;
  }
  else if (is_block_insert(this) && event.GetKeyCode() != WXK_ESCAPE)
  {
    m_command.clear();
    return true;
  }
  else if (
    !event.HasAnyModifiers() &&
    (event.GetKeyCode() == WXK_ESCAPE || event.GetKeyCode() == WXK_BACK ||
     event.GetKeyCode() == WXK_RETURN ||
     event.GetKeyCode() == WXK_NUMPAD_ENTER ||
     (!m_mode.is_visual() && event.GetKeyCode() == WXK_TAB) ||
     (!m_mode.is_insert() &&
      (event.GetKeyCode() == WXK_LEFT || event.GetKeyCode() == WXK_DELETE ||
       event.GetKeyCode() == WXK_DOWN || event.GetKeyCode() == WXK_UP ||
       event.GetKeyCode() == WXK_RIGHT || event.GetKeyCode() == WXK_PAGEUP ||
       event.GetKeyCode() == WXK_PAGEDOWN))))
  {
    if (event.GetKeyCode() == WXK_BACK)
    {
      if (!m_insert_text.empty())
      {
        m_insert_text.pop_back();
      }
    }
    else if (m_command.append_exec(convert_key_event(event)))
    {
      m_command.clear();

      if (!m_mode.is_insert())
      {
        m_insert_command.clear();
      }

      return false;
    }

    return true;
  }
  else if (
    (event.GetModifiers() & wxMOD_CONTROL) && event.GetKeyCode() != WXK_NONE)
  {
    if (const auto& it = get_macros()
                           .get_keys_map(macros::key_t::KEY_CONTROL)
                           .find(event.GetKeyCode());
        it != get_macros().get_keys_map(macros::key_t::KEY_CONTROL).end())
    {
      command(it->second);
      return false;
    }

    return true;
  }
  else if ((event.GetModifiers() & wxMOD_ALT) && event.GetKeyCode() != WXK_NONE)
  {
    if (!m_mode.is_command())
    {
      command(esc());
    }

    if (const auto& it = get_macros()
                           .get_keys_map(macros::key_t::KEY_ALT)
                           .find(event.GetKeyCode());
        it != get_macros().get_keys_map(macros::key_t::KEY_ALT).end())
    {
      command(it->second);
      return false;
    }

    return true;
  }
  else
  {
    return true;
  }
}

bool wex::vi::other_command(std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  filter_count(command);

  if (const auto& it = std::find_if(
        m_other_commands.begin(),
        m_other_commands.end(),
        [command](auto const& e)
        {
          if (!isalpha(e.first.front()))
          {
            return std::any_of(
              e.first.begin(),
              e.first.end(),
              [command](const auto& p)
              {
                return p == command.front();
              });
          }
          else
          {
            return e.first == command.substr(0, e.first.size());
          }
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

bool wex::vi::parse_command(std::string& command)
{
  const auto org(command);

  if (const auto& it = get_macros().get_keys_map().find(command.front());
      it != get_macros().get_keys_map().end())
  {
    command = it->second;
  }

  m_count = 1;

  if (command.front() == '"')
  {
    if (command.size() < 2)
      return false;
    set_register(command[1]);
    get_macros().record(command);
    command.erase(0, 2);
  }
  else if (command.front() == ':')
  {
    return ex::command(command);
  }
  else
  {
    filter_count(command);
  }

  if (command.empty())
  {
    return false;
  }

  const motion_t motion = get_motion(command);

  bool check_other = true;

  switch (command.size())
  {
    case 1:
      if (
        m_mode.is_visual() &&
        (motion == motion_t::CHANGE || motion == motion_t::DEL ||
         motion == motion_t::YANK))
      {
        if (motion == motion_t::CHANGE)
        {
          m_mode.escape();
          std::string s("i");
          m_mode.transition(s);
          command.erase(0, 1);
        }
        else
        {
          command.erase(0, 1);
          m_mode.escape();
        }
      }
      else if (m_mode.transition(command))
      {
        check_other = false;
      }
      else
      {
        m_insert_command.clear();
      }
      break;

    default:
      if (other_command(command))
      {
        return true;
      }

      if (motion > motion_t::G_aa && motion < motion_t::G_ZZ)
      {
        if (command.size() > 2)
        {
          command.erase(0, 2);
        }
      }
      else
      {
        switch (motion)
        {
          case motion_t::CHANGE:
            m_mode.transition(command);
            break;

          case motion_t::DEL:
          case motion_t::YANK:
            command.erase(0, 1);
            break;

          case motion_t::NAVIGATE:
            if (m_mode.transition(command))
            {
              check_other = false;
            }
            else
            {
              m_insert_command.clear();
            }
            break;

          default:
            // do nothing
            break;
        }
      }
  }

  if (
    check_other && !motion_command(motion, command) && !other_command(command))
  {
    return false;
  }
  else
  {
    set_register(0);
  }

  if (!command.empty())
  {
    if (m_mode.is_insert())
    {
      return insert_mode(command);
    }
    else if (command != org)
    {
      return parse_command(command);
    }
    else
    {
      return false;
    }
  }

  return true;
}

bool wex::vi::put(bool after)
{
  if (register_text().empty())
  {
    return false;
  }

  // do not trim
  const bool yanked_lines = (get_number_of_lines(register_text(), false) > 1);

  if (yanked_lines)
  {
    if (after)
    {
      if (
        get_stc()->GetColumn(get_stc()->GetCurrentPos()) > 0 ||
        get_stc()->GetSelectedText().empty())
      {
        get_stc()->LineDown();
      }
    }

    get_stc()->Home();
  }

  get_stc()->add_text(register_text());

  if (yanked_lines && after)
  {
    get_stc()->LineUp();
  }

  return true;
}

void wex::vi::set_last_command(const std::string& command)
{
  size_t first = 0;

  if (regex v("^([1-9][0-9]*)(.*)"); v.match(command) == 2)
  {
    first = v[0].size(); // skip a possible leading count
  }

  if (const auto& it = std::find(
        m_last_commands.begin(),
        m_last_commands.end(),
        command.substr(first, 1));
      it != m_last_commands.end())
  {
    if (command != "gg")
    {
      m_last_command = command;
      log::trace("last command") << m_last_command;
    }
  }
}

void wex::vi::visual_extend(int begin_pos, int end_pos) const
{
  if (begin_pos == wxSTC_INVALID_POSITION || end_pos == wxSTC_INVALID_POSITION)
  {
    return;
  }

  switch (m_mode.get())
  {
    case vi_mode::state_t::VISUAL:
      get_stc()->SetSelection(begin_pos, end_pos);
      break;

    case vi_mode::state_t::VISUAL_LINE:
      if (begin_pos < end_pos)
      {
        get_stc()->SetSelection(
          get_stc()->PositionFromLine(get_stc()->LineFromPosition(begin_pos)),
          get_stc()->PositionFromLine(
            get_stc()->LineFromPosition(end_pos) + 1));
      }
      else
      {
        get_stc()->SetSelection(
          get_stc()->PositionFromLine(get_stc()->LineFromPosition(end_pos)),
          get_stc()->PositionFromLine(
            get_stc()->LineFromPosition(begin_pos) + 1));
      }
      break;

    case vi_mode::state_t::VISUAL_BLOCK:
      if (begin_pos < end_pos)
      {
        while (get_stc()->GetCurrentPos() < end_pos)
        {
          get_stc()->CharRightRectExtend();
        }
      }
      else
      {
        while (get_stc()->GetCurrentPos() > begin_pos)
        {
          get_stc()->CharLeftRectExtend();
        }
      }
      break;

    default:
      break;
  }
}

void wex::vi::yank_range(int start)
{
  if (auto end = get_stc()->GetCurrentPos(); end - start > 0)
  {
    get_stc()->CopyRange(start, end - start);
    get_stc()->SetSelection(start, end);
  }
  else
  {
    // reposition end at start of selection
    if (!get_stc()->GetSelectedText().empty())
    {
      end = get_stc()->GetSelectionStart();
    }
    else
    {
      end--;
    }

    get_stc()->CopyRange(end, start - end);
    get_stc()->SetSelection(end, start);
  }

  m_mode.escape();

  if (!register_name())
  {
    set_register_yank(get_stc()->get_selected_text());
  }
  else
  {
    get_macros().set_register(register_name(), get_stc()->get_selected_text());
    get_stc()->SelectNone();
  }
}
