////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <stdio.h>
#include <string.h>
#include <wex/addressrange.h>
#include <wex/core.h>
#include <wex/ex-stream.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>

#include "ex-stream-line.h"

namespace wex
{
  bool copy(file* from, file* to)
  {
    if (from->stream().bad() || to->stream().bad())
    {
      log("ex stream copy") << from->stream().bad() << to->stream().bad();
      return false;
    }

    to->close();
    to->open(std::ios_base::out);

    from->close();
    from->open(std::ios_base::in);

    to->stream() << from->stream().rdbuf();

    to->close();
    to->open();

    from->close();
    std::remove(from->get_filename().string().c_str());
    from->open(std::ios_base::out);

    return true;
  };

  const std::string tmp_filename()
  {
    char _tmp_filename[L_tmpnam];
    tmpnam(_tmp_filename);
    return std::string(_tmp_filename);
  }
}; // namespace wex

wex::ex_stream::ex_stream(wex::stc* stc)
  : m_context_size(500)
  , m_line_size(500)
  , m_current_line(new char[m_line_size])
  , m_stc(stc)
{
}

wex::ex_stream::~ex_stream()
{
  delete[] m_current_line;

  delete m_temp;
  delete m_work;
}

bool wex::ex_stream::erase(const addressrange& range)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream erase")
    << range.get_begin().get_line() << range.get_end().get_line();

  m_stream->seekg(0);

  ex_stream_line sl(ex_stream_line::ACTION_ERASE, range, m_temp);
  int            i = 0;
  char           c;

  while (m_stream->get(c))
  {
    m_current_line[i++] = c;

    if (c == '\n')
    {
      sl.handle(m_current_line, i);
    }
  }

  sl.handle(m_current_line, i);

  m_last_line_no = sl.lines() - sl.actions() - 1;

  if (!copy(m_temp, m_work))
  {
    return false;
  }

  log::trace("ex stream erase") << sl.actions();

  m_stc->get_frame()->show_ex_message(
    std::to_string(sl.actions()) + " fewer lines");

  goto_line(0);

  m_is_modified = true;

  return true;
}

bool wex::ex_stream::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream find") << text;

  auto line_no = m_line_no;
  auto pos     = m_stream->tellg();
  bool found   = false;

  const std::regex r(text);
  const bool       use_regex(find_replace_data::get()->is_regex());

  while (!found && get_next_line())
  {
    if (
      (use_regex && std::regex_search(m_current_line, r)) ||
      (!use_regex && strstr(m_current_line, text.c_str()) != nullptr))
    {
      found = true;
    }
  }

  if (!found)
  {
    m_line_no = line_no;
    m_stream->clear();
    m_stream->seekg(pos);

    m_stc->get_frame()->statustext(
      get_find_result(text, true, true),
      std::string());
  }
  else
  {
    log::trace("ex stream found") << text << m_line_no;
    m_context = m_current_line;
    set_text();
  }

  return found;
}

int wex::ex_stream::get_current_line() const
{
  return m_line_no;
}

int wex::ex_stream::get_line_count() const
{
  return m_last_line_no;
}

int wex::ex_stream::get_line_count_request()
{
  if (m_stream == nullptr)
  {
    return LINE_COUNT_UNKNOWN;
  }

  auto pos = m_stream->tellg();
  m_stream->clear();
  m_stream->seekg(0);
  m_last_line_no = 0;

  char c;
  while (m_stream->get(c))
  {
    if (c == '\n')
    {
      m_last_line_no++;
    }
  }

  m_stream->clear();
  m_stream->seekg(pos);

  return m_last_line_no;
}

bool wex::ex_stream::get_next_line()
{
  if (!m_stream->getline(m_current_line, m_line_size))
  {
    m_last_line_no = m_line_no + 1;
    return false;
  }

  m_line_no++;

  return true;
}

void wex::ex_stream::goto_line(int no)
{
  if (m_stream == nullptr || no == m_line_no)
  {
    return;
  }

  log::trace("ex stream goto_line") << no << m_line_no;

  if (no < m_line_no)
  {
    // currently reset.
    m_line_no = -1;
    m_stream->clear();
    m_stream->seekg(0);
    log::trace("ex stream reset");
    m_context.clear();

    while ((m_line_no < no) && get_next_line())
    {
    }
  }
  else
  {
    while ((no > m_line_no) && get_next_line())
    {
    }
  }

  set_context();
  set_text();
}

bool wex::ex_stream::insert_text(int line, const std::string& text, loc_t loc)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  m_stream->seekg(0);

  char c;
  int  current = 0;
  bool done    = false;

  if (line == 0 && loc == INSERT_BEFORE)
  {
    if (!m_temp->write(text))
    {
      return false;
    }

    done = true;
  }

  while (m_stream->get(c))
  {
    if (c != '\n')
    {
      m_temp->put(c);
    }
    else
    {
      if (current++ == line && !done)
      {
        switch (loc)
        {
          case INSERT_AFTER:
            m_temp->write(c + text);
            break;

          case INSERT_BEFORE:
            m_temp->write(text + c);
            break;
        }
        done = true;
      }
      else
      {
        m_temp->put(c);
      }
    }
  }

  if (!copy(m_temp, m_work))
  {
    return false;
  }

  goto_line(line);

  m_is_modified = true;

  return true;
}

bool wex::ex_stream::join(const addressrange& range)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream join")
    << range.get_begin().get_line() << range.get_end().get_line();

  m_stream->seekg(0);

  ex_stream_line sl(ex_stream_line::ACTION_JOIN, range, m_temp);
  char           c;
  int            i = 0;

  while (m_stream->get(c))
  {
    m_current_line[i++] = c;

    if (c == '\n')
    {
      sl.handle(m_current_line, i);
    }
  }

  sl.handle(m_current_line, i);

  m_last_line_no = sl.lines() - sl.actions() - 1;

  if (!copy(m_temp, m_work))
  {
    return false;
  }

  log::trace("ex stream joins") << sl.actions();

  m_stc->get_frame()->show_ex_message(
    std::to_string(sl.actions()) + " fewer lines");

  goto_line(0);

  m_is_modified = true;

  return true;
}

void wex::ex_stream::set_context()
{
  m_context += (!m_context.empty() ? "\n" : std::string()) + m_current_line;

  if (m_context.size() > m_context_size)
  {
    m_context = m_context.substr(strlen(m_current_line));
  }
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);
  m_stc->SetText(m_context);
  m_stc->DocumentEnd();
  m_stc->EmptyUndoBuffer();
  m_stc->SetSavePoint();
  m_stc->SetReadOnly(true);
  m_stc->use_modification_markers(false);
}

void wex::ex_stream::stream(file& f)
{
  if (!f.is_open())
  {
    return;
  }

  m_file   = &f;
  m_stream = &f.stream();
  f.use_stream();

  m_temp = new file(tmp_filename(), std::ios_base::out);
  m_temp->use_stream();

  m_work = new file(tmp_filename(), std::ios_base::out);
  m_work->use_stream();
  m_stream = &f.stream();

  goto_line(0);
}

bool wex::ex_stream::substitute(
  const addressrange& range,
  const std::string&  find,
  const std::string&  replace)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream substitute")
    << range.get_begin().get_line() << range.get_end().get_line() << find
    << replace;

  m_stream->seekg(0);

  char           c;
  int            i = 0;
  ex_stream_line sl(range, m_temp, find, replace);

  while (m_stream->get(c))
  {
    m_current_line[i++] = c;

    if (c == '\n')
    {
      sl.handle(m_current_line, i);
    }
  }

  sl.handle(m_current_line, i);

  if (!copy(m_temp, m_work))
  {
    return false;
  }

  log::trace("ex stream substitute") << sl.actions();

  m_stc->get_frame()->show_ex_message(
    "Replaced: " + std::to_string(sl.actions()) + " occurrences of: " + find);

  goto_line(0);

  m_is_modified = true;

  return true;
}

bool wex::ex_stream::write()
{
  log::trace("ex stream write");

  if (!copy(m_work, m_file))
  {
    return false;
  }

  m_is_modified = false;

  return true;
}
