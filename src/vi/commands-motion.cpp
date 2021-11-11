////////////////////////////////////////////////////////////////////////////////
// Name:      commands-motion.cpp
// Purpose:   Implementation of wex::vi::commands_motion
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/macros.h>
#include <wex/vi/vi.h>

#include "defs.h"
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

wex::vi::commands_t wex::vi::commands_motion()
{
  return {
    {"h",
     [&](const std::string& command)
     {
       if (get_stc()->GetColumn(get_stc()->GetCurrentPos()) > 0)
         MOTION(Char, Left, false, false);
       return 1;
     }},
    {"j",
     [&](const std::string& command)
     {
       MOTION(Line, Down, false, false);
     }},
    {"k",
     [&](const std::string& command)
     {
       MOTION(Line, Up, false, false);
     }},
    {"l ",
     [&](const std::string& command)
     {
       if (
         command == "l" &&
         get_stc()->GetCurrentPos() >=
           get_stc()->GetLineEndPosition(get_stc()->get_current_line()))
         return 1;
       MOTION(Char, Right, false, false);
     }},
    {"b",
     [&](const std::string& command)
     {
       MOTION(Word, Left, false, false);
     }},
    {"B",
     [&](const std::string& command)
     {
       MOTION(BigWord, Left, false, false);
     }},
    {"e",
     [&](const std::string& command)
     {
       MOTION(Word, RightEnd, false, false);
     }},
    {"E",
     [&](const std::string& command)
     {
       MOTION(BigWord, RightEnd, false, false);
     }},
    {"w",
     [&](const std::string& command)
     {
       MOTION(Word, Right, false, false);
     }},
    {"W",
     [&](const std::string& command)
     {
       MOTION(BigWord, Right, false, false);
     }},
    {"fFtT,;",
     [&](const std::string& command)
     {
       return find_char(command);
     }},
    {"nN",
     [&](const std::string& command)
     {
       REPEAT(if (const std::string find(
                    get_stc()->get_margin_text_click() > 0 ?
                      config("ex-cmd.margin").get(config::strings_t{}).front() :
                      find_replace_data::get()->get_find_string());
                  !get_stc()->find(
                    find,
                    search_flags(),
                    command == "n" == m_search_forward))
              {
                m_command.clear();
                return (size_t)0;
              });
       return (size_t)1;
     }},
    {"G",
     [&](const std::string& command)
     {
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
     [&](const std::string& command)
     {
       get_stc()->goto_line(get_stc()->GetFirstVisibleLine());
       return 1;
     }},
    {"L",
     [&](const std::string& command)
     {
       get_stc()->goto_line(
         get_stc()->GetFirstVisibleLine() + get_stc()->LinesOnScreen() - 1);
       return 1;
     }},
    {"M",
     [&](const std::string& command)
     {
       get_stc()->goto_line(
         get_stc()->GetFirstVisibleLine() + get_stc()->LinesOnScreen() / 2);
       return 1;
     }},
    {"/?",
     [&](const std::string& command)
     {
       return find_command(command);
     }},
    {"\'",
     [&](const std::string& command)
     {
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
     [&](const std::string& command)
     {
       MOTION(Line, Home, false, false);
     }},
    {"^",
     [&](const std::string& command)
     {
       MOTION(VC, Home, false, false);
     }},
    {"[]",
     [&](const std::string& command)
     {
       // related to regex ECMAScript
       REPEAT(if (!get_stc()->find("\\{", search_flags(), command == "]"))
              {
                m_command.clear();
                return 0;
              })
       return 1;
     }},
    {"({",
     [&](const std::string& command)
     {
       MOTION(Para, Up, false, false);
     }},
    {")}",
     [&](const std::string& command)
     {
       MOTION(Para, Down, false, false);
     }},
    {"+",
     [&](const std::string& command)
     {
       MOTION(Line, Down, true, true);
     }},
    {"|",
     [&](const std::string& command)
     {
       get_stc()->GotoPos(
         get_stc()->PositionFromLine(get_stc()->get_current_line()) + m_count -
         1);
       return 1;
     }},
    {"-",
     [&](const std::string& command)
     {
       MOTION(Line, Up, true, true);
     }},
    {"$",
     [&](const std::string& command)
     {
       MOTION(Line, End, false, false);
     }},
    {"%",
     [&](const std::string& command)
     {
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
     [&](const std::string& command)
     {
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
     [&](const std::string& command)
     {
       get_stc()->Home();
       if (command.front() == '_')
         m_count--;
       MOTION(Line, Down, false, false);
     }},
    {k_s(WXK_CONTROL_B),
     [&](const std::string&)
     {
       MOTION(Page, Up, false, false);
     }},
    {k_s(WXK_CONTROL_D),
     [&](const std::string&)
     {
       MOTION(Page, ScrollDown, false, false);
     }},
    {k_s(WXK_CONTROL_E),
     [&](const std::string&)
     {
       MOTION(Line, ScrollDown, false, false);
     }},
    {k_s(WXK_CONTROL_F),
     [&](const std::string&)
     {
       MOTION(Page, Down, false, false);
     }},
    {k_s(WXK_CONTROL_M),
     [&](const std::string&)
     {
       MOTION(Line, Down, false, false);
     }},
    {k_s(WXK_CONTROL_P),
     [&](const std::string&)
     {
       MOTION(Line, Up, false, false);
     }},
    {k_s(WXK_CONTROL_U),
     [&](const std::string&)
     {
       MOTION(Page, ScrollUp, false, false);
     }},
    {k_s(WXK_CONTROL_Y),
     [&](const std::string&)
     {
       MOTION(Line, ScrollUp, false, false);
     }}};
}

size_t wex::vi::find_char(const std::string& command)
{
  if (command.empty())
  {
    return (size_t)0;
  }

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

  REPEAT(if (!get_stc()->find(std::string(1, c), 0, islower(d) > 0))
         {
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
}

size_t wex::vi::find_command(const std::string& command)
{
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

    if (!get_stc()->find(text, search_flags(), m_search_forward))
      return (size_t)0;
    if (get_stc()->get_margin_text_click() == -1)
      find_replace_data::get()->set_find_string(text);

    return command.size();
  }
  else
  {
    return get_stc()->is_visual() &&
               frame()->show_ex_command(
                 get_stc(),
                 command + (m_mode.is_visual() ? "'<,'>" : "")) ?
             command.size() :
             (size_t)0;
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
