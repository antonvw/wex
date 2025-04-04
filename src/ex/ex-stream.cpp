////////////////////////////////////////////////////////////////////////////////
// Name:      ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/temp-filename.h>
#include <wex/data/find.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#include "ex-stream-line.h"

#define STREAM_LINE_ON_CHAR()                                                  \
  {                                                                            \
    if (m_stream == nullptr || !range.is_ok())                                 \
    {                                                                          \
      return false;                                                            \
    }                                                                          \
                                                                               \
    m_stream->clear();                                                         \
    m_stream->seekg(0);                                                        \
                                                                               \
    size_t i = 0;                                                              \
    char   c;                                                                  \
                                                                               \
    while (m_stream->get(c))                                                   \
    {                                                                          \
      m_current_line[i++] = c;                                                 \
                                                                               \
      if (c == '\n' || i == m_line_size_requested)                             \
      {                                                                        \
        if (sl.handle(m_current_line, i) == wex::ex_stream_line::HANDLE_STOP)  \
        {                                                                      \
          break;                                                               \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    sl.handle(m_current_line, i);                                              \
                                                                               \
    m_stream->clear();                                                         \
                                                                               \
    if (sl.actions() > 0 && sl.is_write() && !copy(m_temp, m_work))            \
    {                                                                          \
      return false;                                                            \
    }                                                                          \
  }

// boost::regex (1.85) has better performance than std::regex (llvm-17):
/*
on file with 1000000 filled lines without xxxx
std::regex:
2025-04-02 18:29:50.860090 [trace] ex command: :/xxxx/
2025-04-02 18:30:32.658455 [trace] address: /xxxx/ line 0 ...
boost::regex:
2025-04-02 19:26:24.884504 [trace] ex command: :/xxxxxxxx/
2025-04-02 19:26:26.417143 [trace] address: /xxxxxxxx/ line 0 ...
*/
#include <boost/regex.hpp>

wex::ex_stream::ex_stream(wex::ex* ex)
  : m_context_lines(40)
  , m_buffer_size(1000000)
  , m_buffer(new char[m_buffer_size])
  , m_ex(ex)
  , m_stc(ex->get_stc())
  , m_function_repeat(
      "stream",
      m_stc,
      [this](wxTimerEvent&)
      {
        if (m_file != nullptr && m_file->check_sync())
        {
          if (m_is_modified)
          {
            log::status("Could not sync") << "file is modified";
            m_function_repeat.activate(false);
          }
          else
          {
            m_file->close();
            m_file->open(std::ios_base::in);
            m_stream->clear();
            m_stream->seekg(0);
          }
        }
      })
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

bool wex::ex_stream::copy(const addressrange& range, const address& dest)
{
  ex_stream_line sl(m_temp, range, dest, ex_stream_line::ACTION_COPY);

  STREAM_LINE_ON_CHAR();

  m_last_line_no = sl.lines() + sl.actions() - 1;

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " added lines");

  return true;
}

bool wex::ex_stream::erase(const addressrange& range)
{
  ex_stream_line sl(m_temp, ex_stream_line::ACTION_ERASE, range);

  STREAM_LINE_ON_CHAR();

  m_last_line_no = sl.lines() - sl.actions() - 1;

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " fewer lines");

  return true;
}

void wex::ex_stream::filter_line(int start, int end, std::streampos spos)
{
  // Copy from start of m_buffer to the current line
  // and set the stream pointer spos to pos after start.
  // start and end always contains a \n
  assert(m_buffer[start] == '\n' && m_buffer[end] == '\n');

  // s       e
  // 3 45678 9
  // \n..... \n
  const size_t sz(end - start - 1);

  memcpy(m_current_line, m_buffer + start + 1, sz);
  m_current_line[sz]  = 0;
  m_line_size_current = sz;

  m_stream->clear();
  m_stream->seekg((size_t)spos + start + 1);
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

  wex::data::find f(
    text,
    m_line_no,
    m_stream->tellg(),
    find_next || (find_flags == -1 && find_replace_data::get()->search_down()));

  f.flags(find_flags);

  return find_data(f);
}

bool wex::ex_stream::find_data(const data::find& f)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  const bool use_regex(m_ex->search_flags() & wxSTC_FIND_REGEXP);

  boost::regex r;

  try
  {
    if (use_regex)
    {
      r = boost::regex(f.text());
    }
  }
  catch (std::exception& e)
  {
    log::status() << e.what();
    log::trace("ex stream find") << e.what();
    return false;
  }

  bool found = false;

  m_stream->clear();

  // Notice we start get..line, and not searching in the current line.
  while (!found && ((f.is_forward() && get_next_line()) ||
                    (!f.is_forward() && get_previous_line())))
  {
    found =
      ((!use_regex && strstr(m_current_line, f.text().c_str()) != nullptr) ||
       (use_regex && boost::regex_search(m_current_line, r)));
  }

  return find_finish(f, found);
}

bool wex::ex_stream::find_finish(const data::find& f, bool& found)
{
  if (!found)
  {
    if (!f.recursive())
    {
      f.statustext();
      f.recursive(true);

      m_stream->clear();

      if (f.is_forward())
      {
        m_line_no = LINE_COUNT_UNKNOWN;
        m_stream->seekg(0);
      }
      else
      {
        m_line_no = m_last_line_no;
        m_stream->seekg(0, std::ios_base::end);
      }

      found = find_data(f);

      if (!found)
      {
        f.statustext();

        m_line_no = f.line_no();
        m_stream->clear();
        m_stream->seekg(f.pos());
      }

      f.recursive(false);
    }
  }
  else
  {
    log::trace("ex stream found")
      << f.text() << "current" << m_line_no << "pos" << (int)m_stream->tellg();

    if (!f.is_forward())
    {
      // not perfect, but for the moment ok
      get_next_line();
    }
  }

  m_line_size_requested = m_line_size_default;

  return found;
}

int wex::ex_stream::get_current_line() const
{
  return m_line_no == LINE_COUNT_UNKNOWN ? 0 : m_line_no;
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
        {
          line_no++;
        }
      }

      if (line_no == 0)
      {
        m_block_mode = true;
      }
    }

    if (m_block_mode)
    {
      line_no += m_buffer_size / m_line_size_default;
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

bool wex::ex_stream::get_lines(
  const addressrange& range,
  const std::string&  flags)
{
  ex_stream_line sl(m_temp, ex_stream_line::ACTION_GET, range, flags);

  STREAM_LINE_ON_CHAR();

  m_text = sl.copy();

  return true;
}

bool wex::ex_stream::get_next_line()
{
  if (!m_stream->getline(m_current_line, m_line_size_requested))
  {
    if (m_stream->eof())
    {
      if (m_stream->gcount() > 0)
      {
        m_last_line_no = m_line_no + 1;
      }

      m_line_size_current = m_stream->gcount() - 1;
      log::status("at end-of-file");

      return false;
    }

    m_stream->clear();
    m_block_mode = true;
  }

  m_line_size_current = m_stream->gcount() - 1;
  m_line_no++;

  // the m_stream->eofbit might be set, without the eof() is on

  return true;
}

bool wex::ex_stream::get_previous_line()
{
  auto pos(m_stream->tellg());

  if (static_cast<int>(pos) - static_cast<int>(m_line_size_requested) > 0)
  {
    m_stream->seekg((size_t)pos - m_line_size_requested);
    pos = m_stream->tellg();
  }
  else if (pos > 0)
  {
    m_stream->seekg(0);
    m_line_size_requested = pos;
    pos                   = 0;
  }
  else
  {
    log("get_previous_line") << m_line_size_requested << "at pos 0";
  }

  m_stream->read(m_buffer, m_line_size_requested);

  if (m_stream->gcount() > 0)
  {
    int  end    = m_stream->gcount() - 1;
    bool second = false;

    for (int i = end; i >= 0; i--)
    {
      if (m_buffer[i] == '\n')
      {
        if (!second)
        {
          end    = i;
          second = true;
        }
        else
        {
          if (m_line_no > 0)
          {
            m_line_no--;
          }

          filter_line(i, end, pos);
          return true;
        }
      }
    }

    memcpy(m_current_line, m_buffer, m_stream->gcount());
    m_current_line[m_stream->gcount()] = 0;
    m_line_size_current                = m_stream->gcount() - 1;
    m_stream->clear();
    m_stream->seekg((size_t)pos);

    // There was no newline, this implies block mode.
    if (m_line_size_requested == m_line_size_default)
    {
      if (m_line_no > 0)
      {
        m_line_no--;
      }

      m_block_mode = true;
      return static_cast<int>(m_stream->gcount()) > m_line_size_requested - 1;
    }

    return static_cast<int>(m_stream->gcount()) > m_line_size_requested;
  }

  return false;
}

const std::string* wex::ex_stream::get_work() const
{
  if (m_work == nullptr)
  {
    return nullptr;
  }

  m_work->stream().clear();

  return m_work->read();
}

void wex::ex_stream::goto_line(int no)
{
  if (m_stream == nullptr)
  {
    return;
  }

  m_stream->clear();

  log::status(std::string());
  log::trace("ex stream goto_line")
    << no << "current" << m_line_no << "pos" << (int)m_stream->tellg();

  if (no == 0 || (no < 100 && no < m_line_no))
  {
    m_line_no             = LINE_COUNT_UNKNOWN;
    m_line_size_requested = m_line_size_default;
    m_stream->seekg(0);

    m_stc->SetReadOnly(false);
    m_stc->ClearAll();
    m_stc->SetReadOnly(true);

    while ((m_line_no < no) && get_next_line())
      ;
  }
  else if (
    no == m_line_no &&
    (no < m_last_line_no - 1 || m_last_line_no == LINE_COUNT_UNKNOWN))
  {
  }
  else if (no < m_line_no)
  {
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
    set_text();
  }

  if (m_stream->eof())
  {
    log::status("at end-of-file");
  }
}

bool wex::ex_stream::insert_text(int a, const std::string& text, loc_t loc)
{
  if (a < 0)
  {
    return false;
  }

  const auto         line(loc == loc_t::BEFORE ? a : a + 1);
  const addressrange range(
    m_ex,
    std::to_string(line) + "," + std::to_string(line));

  ex_stream_line sl(m_temp, range, text);

  STREAM_LINE_ON_CHAR();

  goto_line(line);

  return true;
}

bool wex::ex_stream::join(const addressrange& range)
{
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

bool wex::ex_stream::move(const addressrange& range, const address& dest)
{
  ex_stream_line sl(m_temp, range, dest, ex_stream_line::ACTION_MOVE);

  STREAM_LINE_ON_CHAR();

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " moved lines");

  return true;
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);

  int lines = 2;

  if (!m_stc->is_hexmode())
  {
    m_stc->append_text(std::string(m_current_line, m_line_size_current));
  }
  else
  {
    const auto& text(m_stc->get_hexmode_lines(m_current_line));
    lines = get_number_of_lines(text) + 1;
    m_stc->AppendText(text);
  }

  m_stc->AppendText("\n");

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

void wex::ex_stream::stream(file& f, size_t default_line_size)
{
  if (!f.is_open())
  {
    log("file is not open") << f.path();
    return;
  }

  m_line_size_default   = default_line_size;
  m_line_size_requested = default_line_size;
  m_current_line        = new char[m_line_size_requested];

  m_file = &f;
  m_function_repeat.activate();
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
  ex_stream_line sl(m_temp, range, data);

  STREAM_LINE_ON_CHAR();

  m_ex->frame()->show_ex_message(
    "Replaced: " + std::to_string(sl.actions()) +
    " occurrences of: " + data.pattern());

  return true;
}

bool wex::ex_stream::write()
{
  log::trace("ex stream write");

  if (!m_is_modified || !copy(m_work, m_file))
  {
    return false;
  }

  m_is_modified = false;

  log::info("saved") << m_file->path();
  log::status(_("Saved")) << m_file->path();

  return true;
}

bool wex::ex_stream::write(
  const addressrange& range,
  const std::string&  filename,
  bool                append)
{
  log::trace("ex stream write");

  wex::file file(
    path(filename),
    append ? std::ios::out | std::ios_base::app : std::ios::out);

  ex_stream_line sl(&file, ex_stream_line::ACTION_WRITE, range);

  STREAM_LINE_ON_CHAR();

  log::info("saved") << filename;
  log::status(_("Saved")) << filename;

  return true;
}

bool wex::ex_stream::yank(const addressrange& range, char name)
{
  ex_stream_line sl(m_temp, range, name);

  STREAM_LINE_ON_CHAR();

  m_ex->frame()->show_ex_message(std::to_string(sl.actions()) + " yanked");

  return true;
}
