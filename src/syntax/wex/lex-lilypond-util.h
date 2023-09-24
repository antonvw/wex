////////////////////////////////////////////////////////////////////////////////
// Name:      lex-lilypond-util.h
// Purpose:   Implementation of class lilypond
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>

namespace Scintilla
{
/// Offers some general methods.
class lilypond
{
public:
  /// Static interface

  static bool is_letter(int ch);

  /// Other methods

  /// Constructor.
  lilypond(LexAccessor& s)
    : m_styler(s)
    , m_words{
        "align",
        "alignat",
        "flalign",
        "gather",
        "multiline",
        "displaymath",
        "eqnarray",
        "equation"}
  {
    ;
  }

  bool is_special(char& ch, Sci_Position& no) const;
  bool is_tag_valid(Sci_Position& i, Sci_Position l) const;

  bool last_word_check(
    Sci_Position                    start,
    const char*                     needle,
    const std::vector<std::string>& next,
    Sci_Position                    length,
    int&                            state) const;
  bool last_word_is(Sci_Position start, const char* needle) const;
  bool last_word_is_match_env(Sci_Position pos) const;

  bool next_not_blank_is(Sci_Position i, char needle) const;

private:
  LexAccessor&                   m_styler;
  const std::vector<std::string> m_words;
};

// inline implementation lex_rfw_access

inline bool lilypond::is_letter(int ch)
{
  return IsASCII(ch) && isalpha(ch);
}

inline bool lilypond::is_special(char& ch, Sci_Position& i) const
{
  if (
    (ch == '#') || (ch == '$') || (ch == '%') || (ch == '&') || (ch == '_') ||
    (ch == '{') || (ch == '}') || (ch == ' '))
  {
    m_styler.ColourTo(i + 1, SCE_L_SPECIAL);
    i++;
    ch = m_styler.SafeGetCharAt(i + 1);
    return true;
  }

  return false;
}

inline bool lilypond::is_tag_valid(Sci_Position& i, Sci_Position l) const
{
  while (i < l)
  {
    if (m_styler.SafeGetCharAt(i) == '{')
    {
      while (i < l)
      {
        i++;
        if (m_styler.SafeGetCharAt(i) == '}')
        {
          return true;
        }
        else if (
          !is_letter(m_styler.SafeGetCharAt(i)) &&
          m_styler.SafeGetCharAt(i) != '*')
        {
          return false;
        }
      }
    }
    else if (!isblank(m_styler.SafeGetCharAt(i)))
    {
      return false;
    }
    i++;
  }
  return false;
}

inline bool lilypond::next_not_blank_is(Sci_Position i, char needle) const
{
  while (i < m_styler.Length())
  {
    if (char ch = m_styler.SafeGetCharAt(i); !isspace(ch) && ch != '*')
    {
      return ch == needle;
    }
    i++;
  }
  return false;
}

inline bool lilypond::last_word_check(
  Sci_Position                    start,
  const char*                     needle,
  const std::vector<std::string>& checks,
  Sci_Position                    lengthDoc,
  int&                            state) const
{
  Sci_Position match = start + 3;

  if (last_word_is(match, needle))
  {
    match++;

    if (is_tag_valid(match, lengthDoc))
    {
      if (checks.empty())
      {
        if (last_word_is_match_env(match))
        {
          state = SCE_L_COMMAND;
          return true;
        }
      }
      else
      {
        return std::any_of(
          checks.begin(),
          checks.end(),
          [this, &match, &state, &start](const auto& w)
          {
            if (last_word_is(match, std::string("{" + w + "}").c_str()))
            {
              m_styler.ColourTo(start - 1, state);
              state = SCE_L_COMMAND;
              return true;
            }
            return false;
          });
      }
    }
  }

  return false;
}

inline bool lilypond::last_word_is(Sci_Position start, const char* needle) const
{
  const Sci_PositionU l   = static_cast<Sci_PositionU>(strlen(needle));
  const Sci_Position  ini = start - l + 1;

  char s[32];

  Sci_PositionU i = 0;
  while (i < l && i < 31)
  {
    s[i] = m_styler.SafeGetCharAt(ini + i);
    i++;
  }
  s[i] = '\0';

  return (strcmp(s, needle) == 0);
}

inline bool lilypond::last_word_is_match_env(Sci_Position pos) const
{
  Sci_Position i, j;
  char         s[32];

  if (m_styler.SafeGetCharAt(pos) != '}')
    return false;

  for (i = pos - 1; i >= 0; --i)
  {
    if (m_styler.SafeGetCharAt(i) == '{')
      break;
    if (pos - i >= 20)
      return false;
  }

  if (i < 0 || i == pos - 1)
    return false;

  ++i;
  for (j = 0; i + j < pos; ++j)
    s[j] = m_styler.SafeGetCharAt(i + j);

  s[j] = '\0';
  if (j == 0)
    return false;

  if (s[j - 1] == '*')
    s[--j] = '\0';

  return std::any_of(
    m_words.begin(),
    m_words.end(),
    [&s](const auto& w)
    {
      return w == s;
    });

  return false;
}
}; // namespace Scintilla
