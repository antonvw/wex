////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/factory/stc.h>

void wex::factory::stc::bind_wx()
{
  wex::bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        Copy();
      },
      wxID_COPY},

     {[=, this](wxCommandEvent& event)
      {
        Cut();
      },
      wxID_CUT},

     {[=, this](wxCommandEvent& event)
      {
        // do nothing, to eat the event (for ex commandline)
      },
      wxID_JUMP_TO},

     {[=, this](wxCommandEvent& event)
      {
        Paste();
      },
      wxID_PASTE},

     {[=, this](wxCommandEvent& event)
      {
        Undo();
      },
      wxID_UNDO},

     {[=, this](wxCommandEvent& event)
      {
        Redo();
      },
      wxID_REDO},

     {[=, this](wxCommandEvent& event)
      {
        SelectAll();
      },
      wxID_SELECTALL},

     {[=, this](wxCommandEvent& event)
      {
        if (!GetReadOnly() && !is_hexmode())
          Clear();
      },
      wxID_DELETE}});
}

void wex::factory::stc::clear(bool set_savepoint)
{
  SetReadOnly(false);

  ClearAll();

  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }
}

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

#define FIND_TEXT(FROM, TO)                                               \
  if (const auto pos = FindText(GetCurrentPos(), TO, text, find_flags);   \
      pos != wxSTC_INVALID_POSITION)                                      \
  {                                                                       \
    SetSelection(pos, pos + text.size());                                 \
    return true;                                                          \
  }                                                                       \
                                                                          \
  if (const auto pos = FindText(FROM, GetCurrentPos(), text, find_flags); \
      pos != wxSTC_INVALID_POSITION)                                      \
  {                                                                       \
    SetSelection(pos, pos + text.size());                                 \
    return true;                                                          \
  }

bool wex::factory::stc::find(
  const std::string& text,
  int                find_flags,
  bool               find_next)
{
  if (find_next)
  {
    FIND_TEXT(0, GetTextLength())
  }
  else
  {
    FIND_TEXT(GetTextLength() - 1, 0)
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
  const auto& b(const_cast<stc*>(this)->GetSelectedTextRaw());

  return b.length() == 0 ? std::string() :
                           std::string(b.data(), b.length() - 1);
}

void wex::factory::stc::goto_line(int line)
{
  GotoLine(line);
  EnsureVisible(line);
  EnsureCaretVisible();
}

std::string wex::factory::stc::margin_get_revision_id() const
{
  std::string revision(MarginGetText(m_margin_text_click));
  return get_word(revision);
}

void wex::factory::stc::reset_margins(margin_t type)
{
  for (int i = 0; i < type.size(); ++i)
  {
    if (type.test(i))
    {
      SetMarginWidth(i, 0);

      if (i == MARGIN_TEXT)
      {
        m_margin_text_is_shown = false;
      }
    }
  }
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
  // For now same as WordRightEndRectExtend, there is no
  // WordRightEndPosition.
  const auto repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();

  for (auto i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}

void wex::factory::stc::WordRightEndRectExtend()
{
  const auto repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();

  for (auto i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}
