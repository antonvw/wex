////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw-access.h
// Purpose:   Declaration of Scintilla::lex_rfw_access class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stack>

#include "lex-rfw-defs.h"

namespace Scintilla
{
/// The robotframework lexer access class.
/// It is compiled during wxWidgets LexBash compiling,
/// and uses c++11.
class lex_rfw_access
{
public:
  /// Default constructor, sets LexAccessor and line.
  lex_rfw_access(LexAccessor& styler, int line = -1)
    : m_line(line)
    , m_styler(styler)
  {
    ;
  };

  /// Returns count.
  int count() const { return m_count; }

  /// Decreases count.
  void decrease() { m_count--; }

  /// Returns down.
  int down() const { return m_down; }

  /// Performs global scan.
  int glob_scan(StyleContext& sc) const;

  /// Increases count.
  void increase() { m_count++; }

  /// Initializes.
  Sci_Position init(Sci_PositionU startPos) const;

  /// Returns whether this is a comment line.
  bool is_comment_line(int offset = 0) const;

  /// Returns whether this is a piped line.
  bool is_pipe_line(int offset = 0) const;

  /// Returns whether this is a tabbed line.
  bool is_tab_line(int offset = 0) const;

  /// Returns number base.
  int number_base(char* s) const;

  /// Returns translated digit.
  int translate_digit(int ch) const;

  /// Return up.
  int up() const { return m_up; }

protected:
  int opposite(int ch) const;

  int m_count{0}, m_down{0}, m_up{0};

private:
  bool get_line_pos_eol(int offset, char i) const;

  Sci_Position m_line;
  LexAccessor& m_styler;
};

// Class to manage quote pairs (simplified vs LexPerl)
class quote : public lex_rfw_access
{
public:
  /// Constructor.
  quote(LexAccessor& styler, int line = -1)
    : lex_rfw_access(styler, line)
  {
    ;
  };

  /// Opens.
  void open(int u);

  /// Starts.
  void start(int u);
};

// Class to manage quote pairs that nest
class quote_stack : public lex_rfw_access
{
public:
  /// Constructor.
  quote_stack(LexAccessor& styler, int line = -1)
    : lex_rfw_access(styler, line)
  {
    ;
  };

  /// Returns depth.
  size_t depth() const { return m_stack.size(); }

  /// Pops.
  void pop(void);

  /// Pushes.
  void push(int u, int s);

  /// Starts.
  void start(int u, int s);

  /// Returns style.
  int style() const { return m_style; }

private:
  int m_style{0};

  struct stack_t
  {
    int m_count;
    int m_style;
    int m_up;
  };

  std::stack<stack_t> m_stack;
};
}; // namespace Scintilla

// inline implementation lex_rfw_access

inline bool
Scintilla::lex_rfw_access::get_line_pos_eol(int offset, char c) const
{
  Sci_Position pos = m_styler.LineStart(m_line + offset);
  Sci_Position eol = m_styler.LineStart(m_line + 1 + offset) - 1;

  for (Sci_Position i = pos; i < eol; i++)
  {
    char ch = m_styler[i];
    if (ch == c)
      return true;
    else if (c != '\t' && ch != ' ' && ch != '\t')
      return false;
  }

  return false;
}

inline int Scintilla::lex_rfw_access::glob_scan(StyleContext& sc) const
{
  // forward scan for a glob-like (...), no whitespace allowed
  int c, sLen = 0;
  while ((c = sc.GetRelativeCharacter(++sLen)) != 0)
  {
    if (IsASpace(c))
    {
      return 0;
    }
    else if (c == ')')
    {
      return sLen;
    }
  }

  return 0;
}

inline Sci_Position
Scintilla::lex_rfw_access::init(Sci_PositionU startPos) const
{
  // Always backtracks to the start of a line that is not a continuation
  // of the previous line (i.e. start of a rfw command segment)
  Sci_Position ln = m_styler.GetLine(startPos);

  if (ln > 0 && startPos == static_cast<Sci_PositionU>(m_styler.LineStart(ln)))
    ln--;

  for (;;)
  {
    startPos = m_styler.LineStart(ln);
    if (ln == 0 || m_styler.GetLineState(ln) == RFW_CMD_START)
      break;
    ln--;
  }

  return ln;
}

inline bool Scintilla::lex_rfw_access::is_comment_line(int offset) const
{
  return get_line_pos_eol(offset, '#');
}

inline bool Scintilla::lex_rfw_access::is_pipe_line(int offset) const
{
  return get_line_pos_eol(offset, '|');
}

inline bool Scintilla::lex_rfw_access::is_tab_line(int offset) const
{
  return get_line_pos_eol(offset, '\t');
}

inline int Scintilla::lex_rfw_access::number_base(char* s) const
{
  int i    = 0;
  int base = 0;

  while (*s)
  {
    base = base * 10 + (*s++ - '0');
    i++;
  }

  if (base > 64 || i > 2)
  {
    return RFW_BASE_ERROR;
  }

  return base;
}

inline int Scintilla::lex_rfw_access::opposite(int ch) const
{
  switch (ch)
  {
    case '(':
      return ')';
    case '[':
      return ']';
    case '{':
      return '}';
    case '<':
      return '>';
    default:
      return ch;
  }
}

inline int Scintilla::lex_rfw_access::translate_digit(int ch) const
{
  if (ch >= '0' && ch <= '9')
  {
    return ch - '0';
  }
  else if (ch >= 'a' && ch <= 'z')
  {
    return ch - 'a' + 10;
  }
  else if (ch >= 'A' && ch <= 'Z')
  {
    return ch - 'A' + 36;
  }
  else if (ch == '@')
  {
    return 62;
  }
  else if (ch == '_')
  {
    return 63;
  }

  return RFW_BASE_ERROR;
}

// inline implementation quote

inline void Scintilla::quote::open(int u)
{
  m_count++;

  m_up   = u;
  m_down = opposite(m_up);
}

inline void Scintilla::quote::start(int u)
{
  m_count = 0;
  open(u);
}

// inline implementation quote_stack

inline void Scintilla::quote_stack::pop(void)
{
  if (m_stack.empty())
    return;

  m_stack.pop();

  if (m_stack.empty())
    return;

  m_count = m_stack.top().m_count;
  m_up    = m_stack.top().m_up;
  m_style = m_stack.top().m_style;

  m_down = opposite(m_up);
}

inline void Scintilla::quote_stack::push(int u, int s)
{
  m_stack.push({m_count, m_style, m_up});

  m_count = 1;
  m_up    = u;
  m_style = s;

  m_down = opposite(m_up);
}

inline void Scintilla::quote_stack::start(int u, int s)
{
  m_count = 1;

  m_up    = u;
  m_down  = opposite(m_up);
  m_style = s;
}
