////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/temp-filename.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/ex-stream.h>
#include <wex/vi/ex.h>

#include "ex-stream-line.h"

#define STREAM_LINE_ON_CHAR()                                                 \
  {                                                                           \
    if (!range.is_ok())                                                       \
    {                                                                         \
      return false;                                                           \
    }                                                                         \
                                                                              \
    m_stream->clear();                                                        \
    m_stream->seekg(0);                                                       \
                                                                              \
    int  i = 0;                                                               \
    char c;                                                                   \
                                                                              \
    while (m_stream->get(c))                                                  \
    {                                                                         \
      m_current_line[i++] = c;                                                \
                                                                              \
      if (c == '\n' || i == m_current_line_size)                              \
      {                                                                       \
        if (sl.handle(m_current_line, i) == wex::ex_stream_line::HANDLE_STOP) \
        {                                                                     \
          break;                                                              \
        }                                                                     \
      }                                                                       \
    }                                                                         \
                                                                              \
    sl.handle(m_current_line, i);                                             \
                                                                              \
    m_stream->clear();                                                        \
                                                                              \
    if (sl.action() == ex_stream_line::ACTION_YANK)                           \
    {                                                                         \
      return true;                                                            \
    }                                                                         \
                                                                              \
    if (                                                                      \
      sl.actions() > 0 && sl.action() != ex_stream_line::ACTION_WRITE &&      \
      !copy(m_temp, m_work))                                                  \
    {                                                                         \
      return false;                                                           \
    }                                                                         \
  }

#include <regex>

const int default_line_size = 1000;

wex::ex_stream::ex_stream(wex::ex* ex)
  : m_context_lines(50)
  , m_buffer_size(1000000)
  , m_buffer(new char[m_buffer_size])
  , m_current_line_size(default_line_size)
  , m_current_line(new char[m_current_line_size])
  , m_ex(ex)
  , m_stc(ex->get_stc())
{
}

wex::ex_stream::~ex_stream()
{
  delete[] m_buffer;
  delete[] m_current_line;

  delete m_temp;
  delete m_work;
}

bool wex::ex_stream::copy(file* from, file* to)
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
  std::remove(from->path().string().c_str());
  from->open(std::ios_base::out);

  m_stream      = &to->stream();
  m_is_modified = true;

  return true;
}

bool wex::ex_stream::erase(const addressrange& range)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  ex_stream_line sl(m_temp, ex_stream_line::ACTION_ERASE, range);

  STREAM_LINE_ON_CHAR();

  m_last_line_no = sl.lines() - sl.actions() - 1;

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " fewer lines");

  goto_line(0);

  return true;
}

void wex::ex_stream::filter_line(int start, int end, std::streampos spos)
{
  // m_buffer , now from end of m_buffer search
  // backwards for newline, this is the m_current_line to handle, and set the
  // stream pointer to position before that newline.
  const size_t sz(end - start);

  strncpy(m_current_line, m_buffer + start + 1, sz);
  m_current_line[sz] = 0;
  m_stream->clear();

  if (spos == 0)
  {
    m_current_line_size = start - 1;
    m_stream->clear();
    m_stream->seekg(0);
  }
  else
  {
    m_stream->seekg((size_t)spos + start);
  }

  if (m_line_no > 0)
  {
    m_line_no--;
  }
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

  const auto line_no = m_line_no;
  const auto pos     = m_stream->tellg();
  const bool use_regex(find_replace_data::get()->is_regex());
  bool       found = false;

  std::regex r;

  try
  {
    if (use_regex)
    {
      r = std::regex(text);
    }
  }
  catch (std::exception& e)
  {
    log(e) << "find";
    return false;
  }

  m_stream->clear();

  // Notice we start get..line, and not searching in the current line.
  while (!found && ((find_next && get_next_line()) ||
                    (!find_next && get_previous_line())))
  {
    if (
      (!use_regex && strstr(m_current_line, text.c_str()) != nullptr) ||
      (use_regex && std::regex_search(m_current_line, r)))
    {
      found = true;
    }
  }

  m_current_line_size = default_line_size;

  if (!found)
  {
    m_line_no = line_no;
    m_stream->clear();
    m_stream->seekg(pos);

    m_ex->frame()->statustext(get_find_result(text, true, true), std::string());
  }
  else
  {
    log::trace("ex stream found") << text << "current" << m_line_no;
    find_replace_data::get()->set_find_string(text);
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

  const auto pos = m_stream->tellg();

  m_stream->clear();
  m_stream->seekg(0);

  int line_no = 0;

  while (!m_stream->eof())
  {
    m_stream->read(m_buffer, m_buffer_size);

    const int count = m_stream->gcount();

    if (!m_block_mode)
    {
      for (int i = 0; i < count; i++)
      {
        if (m_buffer[i] == '\n')
          line_no++;
      }

      if (line_no == 0)
      {
        m_block_mode = true;
      }
    }

    if (m_block_mode)
    {
      line_no += m_buffer_size / default_line_size;
    }
  }

  if (m_buffer[m_stream->gcount() - 1] != '\n')
  {
    line_no++;
  }

  m_last_line_no = line_no;

  m_stream->clear();
  m_stream->seekg(pos);

  return m_last_line_no;
}

bool wex::ex_stream::get_next_line()
{
  if (!m_stream->getline(m_current_line, m_current_line_size))
  {
    if (m_stream->eof())
    {
      if (m_stream->gcount() > 0)
      {
        m_last_line_no = m_line_no + 1;
      }

      log::trace("ex stream eof at") << m_last_line_no;

      return false;
    }
    else
    {
      m_stream->clear();
      m_block_mode = true;
    }
  }

  m_line_no++;

  return true;
}

bool wex::ex_stream::get_previous_line()
{
  auto pos(m_stream->tellg());

  if (static_cast<int>(pos) - static_cast<int>(m_current_line_size) > 0)
  {
    m_stream->seekg((size_t)pos - m_current_line_size);
    pos = m_stream->tellg();
  }
  else
  {
    pos = 0;
    m_stream->seekg(0);
  }

  m_stream->read(m_buffer, m_current_line_size);

  if (m_stream->gcount() > 0)
  {
    const int end = m_stream->gcount() - 1;

    for (int i = end; i >= 0; i--)
    {
      if (m_buffer[i] == '\n')
      {
        filter_line(i, end, pos);
        return true;
      }
    }

    strncpy(m_current_line, m_buffer, m_stream->gcount());
    m_current_line[m_stream->gcount()] = 0;
    m_stream->clear();
    m_stream->seekg((size_t)pos);

    if (m_line_no > 0)
    {
      m_line_no--;
    }

    // There was no newline, this implies block mode.
    if (m_current_line_size == default_line_size)
    {
      m_block_mode = true;
      return static_cast<int>(m_stream->gcount()) > m_current_line_size - 1;
    }
    else
    {
      return static_cast<int>(m_stream->gcount()) > m_current_line_size;
    }
  }

  return false;
}

const std::string* wex::ex_stream::get_work() const
{
  if (m_work == nullptr)
  {
    return nullptr;
  }

  return m_work->read();
}

void wex::ex_stream::goto_line(int no)
{
  if (m_stream == nullptr)
  {
    return;
  }

  log::trace("ex stream goto_line") << no << "current" << m_line_no;

  if (no == m_line_no)
  {
  }
  else if (no == 0 || (no < 100 && no < m_line_no) || no < m_line_no - 1000)
  {
    m_line_no = LINE_COUNT_UNKNOWN;
    m_stream->clear();
    m_stream->seekg(0);

    m_stc->SetReadOnly(false);
    m_stc->ClearAll();
    m_stc->SetReadOnly(true);

    while ((m_line_no < no) && get_next_line())
      ;
  }
  else if (no < m_line_no)
  {
    m_stream->clear();

    while ((no < m_line_no) && get_previous_line())
      ;
  }
  else
  {
    while ((no > m_line_no) && get_next_line())
      ;
  }

  if (m_stream->gcount() > 0)
  {
    log::trace("ex stream goto_line last read")
      << m_stream->gcount() << "current" << m_line_no;
    set_text();
  }

  if (m_stream->eof())
  {
    log::trace("at end-of-file");
  }
}

bool wex::ex_stream::insert_text(
  const address&     a,
  const std::string& text,
  loc_t              loc)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  const auto line(loc == INSERT_BEFORE ? a.get_line() : a.get_line() + 1);
  const addressrange range(
    m_ex,
    std::to_string(line) + "," + std::to_string(line + 1));

  ex_stream_line sl(m_temp, range, text);

  STREAM_LINE_ON_CHAR();

  goto_line(line);

  return true;
}

bool wex::ex_stream::join(const addressrange& range)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  ex_stream_line sl(m_temp, ex_stream_line::ACTION_JOIN, range);

  STREAM_LINE_ON_CHAR();

  m_last_line_no = sl.lines() - sl.actions() - 1;

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " fewer lines");

  goto_line(range.begin().get_line() - 1);

  return true;
}

bool wex::ex_stream::marker_add(char marker, int line)
{
  if (!isascii(marker) || line < 0)
  {
    return false;
  }

  m_markers[marker] = line;

  return true;
}

bool wex::ex_stream::marker_delete(char marker)
{
  if (const auto& it = m_markers.find(marker); it != m_markers.end())
  {
    m_markers.erase(marker);
    return true;
  }

  return false;
}

int wex::ex_stream::marker_line(char marker) const
{
  if (const auto& it = m_markers.find(marker); it != m_markers.end())
  {
    return it->second;
  }

  return LINE_NUMBER_UNKNOWN;
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);

  int lines = 2;

  if (!m_stc->is_hexmode())
  {
    m_stc->AppendText(m_current_line);
    m_stc->AppendText("\n");
  }
  else
  {
    const auto& text(m_stc->get_hexmode_lines(m_current_line));
    lines = get_number_of_lines(text) + 1;
    m_stc->AppendText(text);
    m_stc->AppendText("\n");
  }

  if (m_stc->GetLineCount() > m_context_lines)
  {
    m_stc->DeleteRange(0, m_stc->PositionFromLine(1));
  }

  m_stc->DocumentEnd();
  m_stc->EmptyUndoBuffer();
  m_stc->SetSavePoint();
  m_stc->SetReadOnly(true);
  m_stc->use_modification_markers(false);

  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
  m_stc->set_indicator(
    indicator(wex::data::stc().indicator_no()),
    std::max(m_stc->PositionFromLine(m_stc->GetLineCount() - lines), 0),
    m_stc->GetLineEndPosition(m_stc->GetLineCount() - 1));
}

void wex::ex_stream::stream(file& f)
{
  if (!f.is_open())
  {
    log("file is not open") << f.path();
    return;
  }

  m_file   = &f;
  m_stream = &f.stream();
  f.use_stream();

  m_temp = new file(path(temp_filename().name()), std::ios_base::out);
  m_temp->use_stream();

  m_work = new file(path(temp_filename().name()), std::ios_base::out);
  m_work->use_stream();

  goto_line(0);
}

bool wex::ex_stream::substitute(
  const addressrange&     range,
  const data::substitute& data)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  ex_stream_line sl(m_temp, range, data);

  STREAM_LINE_ON_CHAR();

  m_ex->frame()->show_ex_message(
    "Replaced: " + std::to_string(sl.actions()) +
    " occurrences of: " + data.pattern());

  goto_line(0);

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

bool wex::ex_stream::write(
  const addressrange& range,
  const std::string&  filename,
  bool                append)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  wex::file file(
    path(filename),
    append ? std::ios::out | std::ios_base::app : std::ios::out);

  ex_stream_line sl(&file, ex_stream_line::ACTION_WRITE, range);

  STREAM_LINE_ON_CHAR();

  return true;
}

bool wex::ex_stream::yank(const addressrange& range, char name)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  ex_stream_line sl(m_temp, range, name);

  STREAM_LINE_ON_CHAR();

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " yanked");

  return true;
}
