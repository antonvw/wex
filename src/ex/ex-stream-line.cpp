////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/ui/frd.h>

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
    case ex_stream_line::ACTION_YANK:
      return "yanked";

    default:
      assert(0);
      return std::string();
  };
};
}; // namespace wex

wex::ex_stream_line::ex_stream_line(
  file*               work,
  action_t            type,
  const addressrange& range)
  : m_action(type)
  , m_file(work)
  , m_begin(range.begin().get_line() - 1)
  , m_end(
      type != ACTION_JOIN ? range.end().get_line() - 1 :
                            range.end().get_line() - 2)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const std::string&  text)
  : m_action(ACTION_INSERT)
  , m_file(work)
  , m_text(text)
  , m_begin(range.begin().get_line() - 1)
  , m_end(range.end().get_line() - 1)
{
}

wex::ex_stream_line::ex_stream_line(
  file*                   work,
  const addressrange&     range,
  const data::substitute& data)
  : m_action(ACTION_SUBSTITUTE)
  , m_file(work)
  , m_data(data)
  , m_begin(range.begin().get_line() - 1)
  , m_end(range.end().get_line() - 1)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  char                name)
  : m_action(ACTION_YANK)
  , m_file(work)
  , m_begin(range.begin().get_line() - 1)
  , m_end(range.end().get_line() - 1)
  , m_register(name)
{
  ex::get_macros().set_register(m_register, std::string());
}

wex::ex_stream_line::~ex_stream_line()
{
  log::trace("ex stream " + action_name(m_action))
    << m_actions << m_begin << m_end << m_data.pattern()
    << m_data.replacement();
}

wex::ex_stream_line::handle_t wex::ex_stream_line::handle(char* line, int& pos)
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
        handle_substitute(line, pos);
        break;

      case ACTION_WRITE:
        m_actions++;
        m_file->write(line, pos);
        break;

      case ACTION_YANK:
        ex::get_macros().set_register(
          m_register,
          ex::get_macros().get_register(m_register) + std::string(line, pos));
        m_actions++;
        break;

      default:
        assert(0);
        break;
    }
  }
  else
  {
    switch (m_action)
    {
      case ACTION_WRITE:
        break;

      case ACTION_YANK:
        if (m_line > m_end)
        {
          return HANDLE_STOP;
        }

      default:
        m_file->write(line, pos);
    }
  }

  pos = 0;
  m_line++;

  return HANDLE_CONTINUE;
}

void wex::ex_stream_line::handle_substitute(char* line, int& pos)
{
  std::string text(line, pos);

  // if regex matches replace text with replacement
  if (regex r(m_data.pattern()); r.search(text) != -1)
  {
    r.replace(text, m_data.replacement());
    m_actions++;
  }

  m_file->write(text);
}
