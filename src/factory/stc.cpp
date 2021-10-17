////////////////////////////////////////////////////////////////////////////////
// Name:      factory/stc.cpp
// Purpose:   Implementation of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/stc.h>

const std::string wex::factory::stc::eol() const
{
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR:
      return "\r";
    case wxSTC_EOL_CRLF:
      return "\r\n";
    case wxSTC_EOL_LF:
      return "\n";
    default:
      assert(0);
      break;
  }

  return "\r\n";
}

bool wex::factory::stc::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  if (find_next)
  {
    if (const auto pos =
          FindText(GetCurrentPos(), GetTextLength(), text, find_flags);
        pos != wxSTC_INVALID_POSITION)
    {
      SetSelection(pos, pos + text.size());
      return true;
    }

    if (const auto pos = FindText(0, GetCurrentPos(), text, find_flags);
        pos != wxSTC_INVALID_POSITION)
    {
      SetSelection(pos, pos + text.size());
      return true;
    }
  }
  else
  {
    if (const auto pos = FindText(GetCurrentPos(), 0, text, find_flags);
        pos != wxSTC_INVALID_POSITION)
    {
      SetSelection(pos, pos + text.size());
      return true;
    }
    if (const auto pos =
          FindText(GetTextLength() - 1, GetCurrentPos(), text, find_flags);
        pos != wxSTC_INVALID_POSITION)
    {
      SetSelection(pos, pos + text.size());
      return true;
    }
  }

  return false;
}

size_t wex::factory::stc::get_fold_level() const
{
  if (const int level(
        (GetFoldLevel(get_current_line()) & wxSTC_FOLDLEVELNUMBERMASK) -
        wxSTC_FOLDLEVELBASE);
      level > 0)
  {
    return level;
  }
  else
  {
    return 0;
  }
}

const std::string wex::factory::stc::get_selected_text() const
{
  const wxCharBuffer& b(const_cast<stc*>(this)->GetSelectedTextRaw());

  return b.length() == 0 ? std::string() :
                           std::string(b.data(), b.length() - 1);
}

void wex::factory::stc::goto_line(int line)
{
  GotoLine(line);
  EnsureVisible(line);
  EnsureCaretVisible();
}

#define BIGWORD(DIRECTION)                                     \
  int c      = GetCharAt(GetCurrentPos());                     \
  int offset = strncmp((#DIRECTION), "Left", 4) == 0 ? -1 : 0; \
  while (isspace(c) && GetCurrentPos() > 0 &&                  \
         GetCurrentPos() < GetTextLength())                    \
  {                                                            \
    Char##DIRECTION();                                         \
    c = GetCharAt(GetCurrentPos() + offset);                   \
  }                                                            \
  while (!isspace(c) && GetCurrentPos() > 0 &&                 \
         GetCurrentPos() < GetTextLength())                    \
  {                                                            \
    Char##DIRECTION();                                         \
    c = GetCharAt(GetCurrentPos() + offset);                   \
  }

void wex::factory::stc::BigWordLeft()
{
  BIGWORD(Left);
}

void wex::factory::stc::BigWordLeftExtend()
{
  BIGWORD(LeftExtend);
}

void wex::factory::stc::BigWordLeftRectExtend()
{
  BIGWORD(LeftRectExtend);
}

void wex::factory::stc::BigWordRight()
{
  BIGWORD(Right);
}

void wex::factory::stc::BigWordRightEnd()
{
  BIGWORD(Right);
}

void wex::factory::stc::BigWordRightEndExtend()
{
  BIGWORD(RightExtend);
}

void wex::factory::stc::BigWordRightEndRectExtend()
{
  BIGWORD(RightRectExtend);
}

void wex::factory::stc::BigWordRightExtend()
{
  BIGWORD(RightExtend);
}

void wex::factory::stc::BigWordRightRectExtend()
{
  BIGWORD(RightRectExtend);
}

void wex::factory::stc::PageScrollDown()
{
  LineScroll(0, 10);
}

void wex::factory::stc::PageScrollUp()
{
  LineScroll(0, -10);
}

void wex::factory::stc::WordLeftRectExtend()
{
  const auto repeat =
    GetCurrentPos() - WordStartPosition(GetCurrentPos(), false);

  for (auto i = 0; i < repeat; i++)
  {
    CharLeftRectExtend();
  }
}

void wex::factory::stc::WordRightRectExtend()
{
  const auto repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();

  for (auto i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}
