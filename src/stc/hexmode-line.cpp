////////////////////////////////////////////////////////////////////////////////
// Name:      stc/hexmode-line.cpp
// Purpose:   Implementation of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/hex.hpp>
#include <charconv>

#include <wex/factory/stc.h>

#include "hexmode-line.h"

wex::hexmode_line::hexmode_line(wex::hexmode* hex)
  : m_line(hex->get_stc()->GetCurLine())
  , m_line_no(hex->get_stc()->get_current_line())
  , m_column_no(hex->get_stc()->GetColumn(hex->get_stc()->GetCurrentPos()))
  , m_hex(hex)
  , m_start_ascii_field(hex->each_hex_field() * hex->bytes_per_line())
{
  assert(m_hex->is_active());
}

wex::hexmode_line::hexmode_line(
  hexmode* hex,
  int      pos_or_offset,
  bool     is_position)
  : m_hex(hex)
  , m_start_ascii_field(hex->each_hex_field() * hex->bytes_per_line())
{
  assert(m_hex->is_active());

  if (is_position)
  {
    const auto pos =
      (pos_or_offset != wxSTC_INVALID_POSITION ?
         pos_or_offset :
         m_hex->get_stc()->GetCurrentPos());

    m_column_no = m_hex->get_stc()->GetColumn(pos);
    m_line_no   = m_hex->get_stc()->LineFromPosition(pos);

    if (
      m_column_no >=
      m_start_ascii_field + static_cast<int>(m_hex->bytes_per_line()))
    {
      m_column_no = m_start_ascii_field;
      m_line_no++;
    }

    m_line = m_hex->get_stc()->GetLine(m_line_no);
  }
  else
  {
    m_hex->get_stc()->goto_line(pos_or_offset >> 4);
    m_hex->get_stc()->SelectNone();
    m_column_no = (pos_or_offset & 0x0f);
    m_line_no   = m_hex->get_stc()->get_current_line();
    m_line      = m_hex->get_stc()->GetLine(m_line_no);
  }
}

int wex::hexmode_line::buffer_index() const
{
  if (
    m_column_no >=
    m_start_ascii_field + static_cast<int>(m_hex->bytes_per_line()))
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (m_column_no >= m_start_ascii_field)
  {
    return convert(m_column_no - m_start_ascii_field);
  }
  else if (m_column_no >= 0)
  {
    if (m_line[m_column_no] != ' ')
    {
      return convert(m_column_no / m_hex->each_hex_field());
    }
  }

  return wxSTC_INVALID_POSITION;
}

bool wex::hexmode_line::erase(int count, bool settext)
{
  const auto index = buffer_index();

  if (
    is_readonly() || index == wxSTC_INVALID_POSITION ||
    (size_t)index >= m_hex->m_buffer.length())
  {
    return false;
  }

  m_hex->m_buffer.erase(index, count);

  if (settext)
  {
    const auto pos(m_hex->get_stc()->GetCurrentPos());
    m_hex->set_text_from_buffer();
    m_hex->get_stc()->SetCurrentPos(pos);
  }

  return true;
}

const std::string wex::hexmode_line::info() const
{
  if (is_hex_field())
  {
    if (const auto& word =
          m_hex->get_stc()->get_word_at_pos(m_hex->get_stc()->GetCurrentPos());
        !word.empty())
    {
      long val;
      std::from_chars(word.data(), word.data() + word.size(), val, 16);
      return std::string("index: ") + std::to_string(buffer_index()) +
             std::string(" ") + std::to_string(val);
    }
  }
  else if (is_ascii_field())
  {
    return std::string("index: ") + std::to_string(buffer_index());
  }

  return std::string();
}

bool wex::hexmode_line::insert(const std::string& text)
{
  const auto index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
  {
    return false;
  }

  if (m_column_no >= m_start_ascii_field)
  {
    const auto pos(m_hex->get_stc()->GetCurrentPos());
    m_hex->m_buffer.insert(index, text);
    m_hex->set_text_from_buffer();

    if (
      m_column_no + text.size() >=
      m_hex->bytes_per_line() + m_start_ascii_field)
    {
      const int line_no =
        m_hex->get_stc()->LineFromPosition(m_hex->get_stc()->GetCurrentPos()) +
        1;
      m_hex->get_stc()->SetCurrentPos(
        m_hex->get_stc()->PositionFromLine(line_no) + m_start_ascii_field);
    }
    else
    {
      m_hex->get_stc()->SetCurrentPos(pos + text.size());
    }

    m_hex->get_stc()->SelectNone();
  }
  else
  {
    if (text.size() != 2 || (!isxdigit(text[0]) && !isxdigit(text[1])))
    {
      return false;
    }

    int val = 0;
    std::from_chars(text.data(), text.data() + 2, val, 16);

    const auto pos(m_hex->get_stc()->GetCurrentPos());
    m_hex->m_buffer.insert(index, 1, val);
    m_hex->set_text_from_buffer();
    m_hex->get_stc()->SetCurrentPos(pos);
  }

  return true;
}

bool wex::hexmode_line::replace(char c)
{
  const auto index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
  {
    return false;
  }

  const auto pos = m_hex->get_stc()->PositionFromLine(m_line_no);

  // Because m_buffer is changed, begin and end undo action
  // cannot be used, as these do not operate on the hex buffer.

  if (is_ascii_field())
  {
    // replace ascii field with value
    m_hex->get_stc()->wxStyledTextCtrl::Replace(
      pos + m_column_no,
      pos + m_column_no + 1,
      c);

    // replace hex field with code
    m_hex->get_stc()->wxStyledTextCtrl::Replace(
      pos + other_field(),
      pos + other_field() + 2,
      boost::algorithm::hex(std::string(1, c)));
  }
  else if (is_hex_field())
  {
    // hex text should be entered.
    if (!isxdigit(c))
    {
      return false;
    }

    // replace hex field with value
    m_hex->get_stc()->wxStyledTextCtrl::Replace(
      pos + m_column_no,
      pos + m_column_no + 1,
      c);

    // replace ascii field with code
    std::string hex;

    if (m_line[m_column_no + 1] == ' ')
    {
      hex += m_line[m_column_no - 1];
      hex += c;
    }
    else
    {
      hex += c;
      hex += m_line[m_column_no];
    }

    int code;
    std::from_chars(hex.data(), hex.data() + hex.size(), code, 16);

    m_hex->get_stc()->wxStyledTextCtrl::Replace(
      pos + other_field(),
      pos + other_field() + 1,
      m_hex->printable(code));

    c = code;
  }
  else
  {
    return false;
  }

  m_hex->m_buffer[index] = c;

  return true;
}

void wex::hexmode_line::replace(const std::string& hex, bool settext)
{
  const auto index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
  {
    return;
  }

  int code;
  std::from_chars(hex.data(), hex.data() + hex.size(), code, 16);
  m_hex->m_buffer[index] = code;

  if (settext)
  {
    m_hex->set_text_from_buffer();
  }
}

void wex::hexmode_line::replace_hex(int value)
{
  const auto index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
  {
    return;
  }

  const auto pos = m_hex->get_stc()->PositionFromLine(m_line_no);

  // replace hex field with value
  m_hex->get_stc()->wxStyledTextCtrl::Replace(
    pos + m_column_no,
    pos + m_column_no + 2,
    boost::algorithm::hex(std::string(1, value)));

  // replace ascii field with code
  m_hex->get_stc()->wxStyledTextCtrl::Replace(
    pos + other_field(),
    pos + other_field() + 1,
    m_hex->printable(value));

  m_hex->m_buffer[index] = value;
}

bool wex::hexmode_line::set_pos() const
{
  if (m_line_no < 0 || m_column_no < 0)
  {
    return false;
  }

  m_hex->get_stc()->SetFocus();
  m_hex->get_stc()->SetCurrentPos(
    m_hex->get_stc()->PositionFromLine(m_line_no) + m_start_ascii_field +
    m_column_no);
  m_hex->get_stc()->SelectNone();

  return true;
}

void wex::hexmode_line::set_pos(const wxKeyEvent& event) const
{
  const int  start = m_hex->get_stc()->PositionFromLine(m_line_no);
  const bool right = (event.GetKeyCode() == WXK_RIGHT);
  const int  pos   = m_hex->get_stc()->GetCurrentPos();

  if (is_hex_field())
  {
    right ? m_hex->get_stc()->SetCurrentPos(pos + 3) :
            m_hex->get_stc()->SetCurrentPos(pos - 3);

    if (m_hex->get_stc()->GetCurrentPos() >= start + m_start_ascii_field)
    {
      m_hex->get_stc()->SetCurrentPos(
        m_hex->get_stc()->PositionFromLine(m_line_no + 1));
    }
    else if (m_hex->get_stc()->GetCurrentPos() < start)
    {
      if (m_line_no > 0)
      {
        m_hex->get_stc()->SetCurrentPos(
          m_hex->get_stc()->PositionFromLine(m_line_no - 1) +
          m_start_ascii_field - m_hex->each_hex_field());
      }
    }
  }
  else
  {
    right ? m_hex->get_stc()->SetCurrentPos(pos + 1) :
            m_hex->get_stc()->SetCurrentPos(pos - 1);

    if (
      m_hex->get_stc()->GetCurrentPos() >=
      start + m_start_ascii_field + static_cast<int>(m_hex->bytes_per_line()))
    {
      m_hex->get_stc()->SetCurrentPos(
        m_hex->get_stc()->PositionFromLine(m_line_no + 1) +
        m_start_ascii_field);
    }
    else if (m_hex->get_stc()->GetCurrentPos() < start + m_start_ascii_field)
    {
      if (m_line_no > 0)
      {
        m_hex->get_stc()->SetCurrentPos(
          m_hex->get_stc()->GetLineEndPosition(m_line_no - 1) - 1);
      }
    }
  }
}
