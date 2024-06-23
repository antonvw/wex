////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream-line.cpp
// Purpose:   Implementation of class wex::ex_stream_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/ui/frd.h>

#include "ex-stream-line.h"

namespace wex
{
const std::unordered_map<ex_stream_line::action_t, std::string>
  ex_stream_line::m_action_names = {
    {ex_stream_line::ACTION_COPY, "copied"},
    {ex_stream_line::ACTION_ERASE, "erased"},
    {ex_stream_line::ACTION_GET, "get"},
    {ex_stream_line::ACTION_INSERT, "inserted"},
    {ex_stream_line::ACTION_JOIN, "joined"},
    {ex_stream_line::ACTION_MOVE, "moved"},
    {ex_stream_line::ACTION_SUBSTITUTE, "substituted"},
    {ex_stream_line::ACTION_WRITE, "written"},
    {ex_stream_line::ACTION_YANK, "yanked"}};
}; // namespace wex

wex::ex_stream_line::ex_stream_line(
  file*                   work,
  action_t                type,
  const addressrange&     range,
  const std::string&      text,
  const data::substitute& data,
  char                    name,
  const address&          dest)
  : m_action(type)
  , m_file(work)
  , m_text(text)
  , m_data(data)
  , m_register(name)
  , m_dest(dest.get_line() - 1)
  , m_begin(range.begin().get_line() - 1)
  , m_end(
      type != ACTION_JOIN ? range.end().get_line() - 1 :
                            range.end().get_line() - 2)
  , m_reflect(
      {REFLECT_ADD("actions", m_actions),
       REFLECT_ADD("from", m_begin),
       REFLECT_ADD("to", m_end),
       REFLECT_ADD("pattern", m_data.pattern()),
       REFLECT_ADD("replacement", m_data.replacement())},
      reflection::log_t::SKIP_EMPTY)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const std::string&  text)
  : ex_stream_line(work, ACTION_INSERT, range, text)
{
}

wex::ex_stream_line::ex_stream_line(
  file*                   work,
  const addressrange&     range,
  const data::substitute& data)
  : ex_stream_line(work, ACTION_SUBSTITUTE, range, std::string(), data)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  const address&      dest,
  action_t            type)
  : ex_stream_line(
      work,
      type,
      range,
      std::string(),
      data::substitute(),
      0,
      dest)
{
}

wex::ex_stream_line::ex_stream_line(
  file*               work,
  const addressrange& range,
  char                name)
  : ex_stream_line(
      work,
      ACTION_YANK,
      range,
      std::string(),
      data::substitute(),
      name)
{
  ex::get_macros().set_register(m_register, std::string());
}

wex::ex_stream_line::~ex_stream_line()
{
  log::trace("ex stream " + m_action_names.at(m_action)) << m_reflect.log();
}

wex::ex_stream_line::handle_t
wex::ex_stream_line::handle(char* line, size_t& pos)
{
  if (m_line >= m_begin && m_line <= m_end)
  {
    switch (m_action)
    {
      case ACTION_COPY:
        m_file->write(std::span{line, pos});
        m_copy.append(line, pos);
        m_actions++;
        break;

      case ACTION_ERASE:
        // skip this line: no write at all
        m_actions++;
        break;

      case ACTION_GET:
        if (m_text.contains("#"))
        {
          append_line_no(m_copy, m_line);
        }
        if (m_text.contains("l"))
        {
          m_copy.append(line, pos - 1);
          m_copy.append("$\n");
        }
        else
        {
          m_copy.append(line, pos);
        }

        m_actions++;
        break;

      case ACTION_INSERT:
        m_actions++;
        m_file->write(m_text);
        m_file->write(std::span{line, pos});
        break;

      case ACTION_JOIN:
        // join: do not write last char, that is \n
        m_actions++;
        m_file->write(std::span{line, pos - 1});
        break;

      case ACTION_MOVE:
        m_copy.append(line, pos);
        m_actions++;
        break;

      case ACTION_SUBSTITUTE:
        handle_substitute(line, pos);
        break;

      case ACTION_WRITE:
        m_actions++;
        m_file->write(std::span{line, pos});
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
      case ACTION_COPY:
      case ACTION_MOVE:
        m_file->write(std::span{line, pos});

        if (m_line > m_dest && !m_copy.empty())
        {
          m_file->write(m_copy);
          m_copy.clear();
        }
        break;

      case ACTION_WRITE:
        break;

      case ACTION_GET:
      case ACTION_YANK:
        if (m_line > m_end)
        {
          return HANDLE_STOP;
        }
        break;

      default:
        m_file->write(std::span{line, pos});
    }
  }

  pos = 0;
  m_line++;

  return HANDLE_CONTINUE;
}

// cppcheck gives incorrect warning here
void wex::ex_stream_line::handle_substitute(char* line, size_t pos)
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
