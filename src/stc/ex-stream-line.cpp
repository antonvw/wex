////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <stdio.h>
#include <string.h>
#include <wex/addressrange.h>
#include <wex/frd.h>
#include <wex/log.h>

#include "ex-stream-line.h"

namespace wex
{
  std::string action_name(ex_stream_line::action_t type)
  {
    switch (type)
    {
      case ex_stream_line::ACTION_ERASE:
        return "erased";
      case ex_stream_line::ACTION_INSERT:
        return "inserted";
      case ex_stream_line::ACTION_JOIN:
        return "joined";
      case ex_stream_line::ACTION_SUBSTITUTE:
        return "substituted";
    };
  };
}; // namespace wex

wex::ex_stream_line::ex_stream_line(
  action_t            type,
  file*               work,
  const addressrange& range)
  : m_action(type)
  , m_range(range)
  , m_file(work)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const std::string&  find,
  const std::string&  replace)
  : m_action(ACTION_SUBSTITUTE)
  , m_range(range)
  , m_file(work)
  , m_find(find)
  , m_replace(replace)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const std::string&  text)
  : m_action(ACTION_INSERT)
  , m_range(range)
  , m_file(work)
  , m_text(text)
{
}

wex::ex_stream_line::~ex_stream_line()
{
  log::trace("ex stream") << action_name(m_action) << m_actions
                          << m_range.get_begin().get_line()
                          << m_range.get_end().get_line() << m_find
                          << m_replace;
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

      case ACTION_INSERT:
        m_actions++;
        m_file->write(m_text);
        m_file->write(line, pos);
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

          m_file->write(text);
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
