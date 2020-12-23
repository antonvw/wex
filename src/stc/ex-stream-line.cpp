////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_lne
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <stdio.h>
#include <string.h>
#include <wex/addressrange.h>
#include <wex/frd.h>

#include "ex-stream-line.h"

wex::ex_stream_line::ex_stream_line(
  action_t            type,
  const addressrange& range,
  file*               work)
  : m_action(type)
  , m_range(range)
  , m_file(work)
{
}

wex::ex_stream_line::ex_stream_line(
  const addressrange& range,
  file*               work,
  const std::string&  find,
  const std::string&  replace)
  : m_action(ACTION_SUBSTITUTE)
  , m_range(range)
  , m_file(work)
  , m_find(find)
  , m_replace(replace)
{
}

void wex::ex_stream_line::handle(char* line, int& pos)
{
  if (
    m_line >= m_range.get_begin().get_line() - 1 &&
    m_line <= m_range.get_end().get_line() - 1)
  {
    switch (m_action)
    {
      case ACTION_ERASE:
        // skip this line: no write at all
        m_actions++;
        break;

      case ACTION_JOIN:
        // join: do not write last char, that is \n
        m_actions++;
        m_file->write(line, pos - 1);
        break;

      case ACTION_SUBSTITUTE:
      {
        // if match writes modified line, else write original line
        const std::regex r(m_find);
        char*            pch;
        std::smatch      m;

        if (find_replace_data::get()->is_regex())
        {
          std::string text(line, pos);

          if (std::regex_search(text, m, r))
          {
            text = std::regex_replace(text, r, m_replace);
            m_actions++;
          }

          m_file->write(text.c_str(), text.size());
        }
        else if ((pch = strstr(line, m_find.c_str())) != nullptr)
        {
          strncpy(pch, m_replace.c_str(), m_replace.size());
          m_file->write(line, pos - m_find.size() + m_replace.size());
          m_actions++;
        }
        else
        {
          m_file->write(line, pos);
        }
      }

      break;

      default:
        assert(0);
        break;
    }
  }
  else
  {
    m_file->write(line, pos);
  }

  pos = 0;
  m_line++;
}
