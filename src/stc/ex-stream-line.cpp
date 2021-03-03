////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <wex/addressrange.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/regex.h>

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
      case ex_stream_line::ACTION_WRITE:
        return "written";

      default:
        assert(0);
        return std::string();
    };
  };
}; // namespace wex

wex::ex_stream_line::ex_stream_line(
  action_t            type,
  file*               work,
  const addressrange& range)
  : m_action(type)
  , m_file(work)
  , m_begin(range.get_begin().get_line() - 1)
  , m_end(range.get_end().get_line() - 1)
{
}

wex::ex_stream_line::ex_stream_line(
  file*                   work,
  const addressrange&     range,
  const data::substitute& data)
  : m_action(ACTION_SUBSTITUTE)
  , m_file(work)
  , m_data(data)
  , m_begin(range.get_begin().get_line() - 1)
  , m_end(range.get_end().get_line() - 1)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const std::string&  text)
  : m_action(ACTION_INSERT)
  , m_file(work)
  , m_text(text)
  , m_begin(range.get_begin().get_line() - 1)
  , m_end(range.get_end().get_line() - 1)
{
}

wex::ex_stream_line::~ex_stream_line()
{
  log::trace("ex stream") << action_name(m_action) << m_actions << m_begin
                          << m_end << m_data.pattern() << m_data.replacement();
}

void wex::ex_stream_line::handle(char* line, int& pos)
{
  if (m_line >= m_begin && m_line <= m_end)
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
        char* pch = line;

        if (find_replace_data::get()->is_regex())
        {
          // if match writes modified line, else write original line
          std::string text(line, pos);

          if (regex r(m_data.pattern()); r.search(text))
          {
            r.replace(text, m_data.replacement());
            m_actions++;
          }

          m_file->write(text);
        }
        else if ((pch = strstr(pch, m_data.pattern().c_str())) != nullptr)
        {
          do
          {
            strncpy(
              pch,
              m_data.replacement().c_str(),
              m_data.replacement().size());

            pch++;
            m_actions++;
            pos += m_data.replacement().size() - m_data.pattern().size();
          } while (m_data.is_global() &&
                   (pch = strstr(pch, m_data.pattern().c_str())) != nullptr);

          m_file->write(line, pos);
        }
        else
        {
          m_file->write(line, pos);
        }
      }

      break;

      case ACTION_WRITE:
        m_actions++;
        m_file->write(line, pos);
        break;

      default:
        assert(0);
        break;
    }
  }
  else if (m_action != ACTION_WRITE)
  {
    m_file->write(line, pos);
  }

  pos = 0;
  m_line++;
}
