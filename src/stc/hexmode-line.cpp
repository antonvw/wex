////////////////////////////////////////////////////////////////////////////////
// Name:      stc/hexmode-line.cpp
// Purpose:   Implementation of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
    const int pos =
      (pos_or_offset != -1 ? pos_or_offset : m_hex->get_stc()->GetCurrentPos());

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
  const int index = buffer_index();

  if (
    is_readonly() || index == wxSTC_INVALID_POSITION ||
    (size_t)index >= m_hex->m_buffer.length())
    return false;

  m_hex->m_buffer.erase(index, count);

  if (settext)
  {
    m_hex->set_text(m_hex->m_buffer);
  }

  return true;
}

const std::string wex::hexmode_line::info() const
{
  if (is_hex_field())
  {
    const std::string word =
      m_hex->get_stc()->get_word_at_pos(m_hex->get_stc()->GetCurrentPos());

    if (!word.empty())
    {
      return std::string("index: ") + std::to_string(buffer_index()) +
             std::string(" ") + std::to_string(std::stol(word, 0, 16));
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
  const int index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
    return false;

  if (m_column_no >= m_start_ascii_field)
  {
    m_hex->m_buffer.insert(index, text);
    m_hex->set_text(m_hex->m_buffer);

    if (
      m_column_no + text.size() >=
      m_hex->bytes_per_line() + m_start_ascii_field)
    {
      int line_no =
        m_hex->get_stc()->LineFromPosition(m_hex->get_stc()->GetCurrentPos()) +
        1;
      m_hex->get_stc()->SetCurrentPos(
        m_hex->get_stc()->PositionFromLine(line_no) + m_start_ascii_field);
    }
    else
    {
      m_hex->get_stc()->SetCurrentPos(
        m_hex->get_stc()->GetCurrentPos() + text.size());
    }

    m_hex->get_stc()->SelectNone();
  }
  else
  {
    if (text.size() != 2 || (!isxdigit(text[0]) && !isxdigit(text[1])))
      return false;

    m_hex->m_buffer.insert(index, 1, std::stoi(text.substr(0, 2), nullptr, 16));
    m_hex->set_text(m_hex->m_buffer);
  }

  return true;
}

bool wex::hexmode_line::replace(char c)
{
  const int index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
    return false;

  const int pos = m_hex->get_stc()->PositionFromLine(m_line_no);

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
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%02X", c);

    m_hex->get_stc()->wxStyledTextCtrl::Replace(
      pos + other_field(),
      pos + other_field() + 2,
      buffer);
  }
  else if (is_hex_field())
  {
    // hex text should be entered.
    if (!isxdigit(c))
      return false;

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

    const int code = std::stoi(hex, nullptr, 16);

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
  const int index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
    return;

  m_hex->m_buffer[index] = std::stoi(hex, nullptr, 16);

  if (settext)
  {
    m_hex->set_text(m_hex->m_buffer);
  }
}

void wex::hexmode_line::replace_hex(int value)
{
  const int index = buffer_index();

  if (is_readonly() || index == wxSTC_INVALID_POSITION)
    return;

  const int pos = m_hex->get_stc()->PositionFromLine(m_line_no);

  char buffer[3];
  snprintf(buffer, sizeof(buffer), "%2X", value);

  // replace hex field with value
  m_hex->get_stc()->wxStyledTextCtrl::Replace(
    pos + m_column_no,
    pos + m_column_no + 2,
    buffer);

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
    return false;

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
