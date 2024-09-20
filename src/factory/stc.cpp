////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::factory::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/factory/stc.h>
#include <wx/app.h>

wex::factory::stc::stc(const wex::data::window& data)
  : wxStyledTextCtrl(
      data.parent() != nullptr ? data.parent() : wxTheApp->GetTopWindow(),
      data.id(),
      data.pos(),
      data.size(),
      data.style(),
      data.name())
  , m_command(this)
{
}

void wex::factory::stc::bind_wx()
{
  wex::bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        Copy();
      },
      wxID_COPY},

     {[=, this](const wxCommandEvent& event)
      {
        Cut();
      },
      wxID_CUT},

     {[=, this](const wxCommandEvent& event)
      {
        // do nothing, to eat the event (for ex commandline)
      },
      wxID_JUMP_TO},

     {[=, this](const wxCommandEvent& event)
      {
        Paste();
      },
      wxID_PASTE},

     {[=, this](const wxCommandEvent& event)
      {
        Undo();
      },
      wxID_UNDO},

     {[=, this](const wxCommandEvent& event)
      {
        Redo();
      },
      wxID_REDO},

     {[=, this](const wxCommandEvent& event)
      {
        SelectAll();
      },
      wxID_SELECTALL},

     {[=, this](const wxCommandEvent& event)
      {
        if (!GetReadOnly() && !is_hexmode())
        {
          Clear();
        }
      },
      wxID_DELETE}});
}

void wex::factory::stc::clear(bool set_savepoint)
{
  const bool restore = !is_visual() && GetReadOnly();

  SetReadOnly(false);

  ClearAll();

  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }

  if (restore)
  {
    SetReadOnly(true);
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

#define FIND_TEXT(FROM, TO)                                                    \
  if (const auto pos = FindText(GetCurrentPos(), TO, text, find_flags);        \
      pos != wxSTC_INVALID_POSITION)                                           \
  {                                                                            \
    SetSelection(pos, pos + text.size());                                      \
    return true;                                                               \
  }                                                                            \
                                                                               \
  if (const auto pos = FindText(FROM, GetCurrentPos(), text, find_flags);      \
      pos != wxSTC_INVALID_POSITION)                                           \
  {                                                                            \
    SetSelection(pos, pos + text.size());                                      \
    return true;                                                               \
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

  return 0;
}

const std::string wex::factory::stc::get_selected_text() const
{
  const auto& b(const_cast<stc*>(this)->GetSelectedTextRaw());

  return b.length() == 0 ? std::string() :
                           std::string(b.data(), b.length() - 1);
}

const std::string wex::factory::stc::get_word_at_pos(int pos) const
{
  const auto word_start = const_cast<stc*>(this)->WordStartPosition(pos, true);
  const auto word_end   = const_cast<stc*>(this)->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const std::string word = const_cast<stc*>(this)
                               ->GetTextRange(word_start, word_start + 1)
                               .ToStdString();

    return !isspace(word[0]) ? word : std::string();
  }

  return const_cast<stc*>(this)
    ->GetTextRange(word_start, word_end)
    .ToStdString();
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

bool wex::factory::stc::position_restore()
{
  if (m_saved_selection_start != -1 && m_saved_selection_end != -1)
  {
    SetSelection(m_saved_selection_start, m_saved_selection_end);
    SetCurrentPos(m_saved_selection_start);
  }
  else if (m_saved_pos != -1)
  {
    SetSelection(m_saved_pos, m_saved_pos);
    SetCurrentPos(m_saved_pos);
  }
  else
  {
    return false;
  }

  EnsureCaretVisible();

  return true;
}

void wex::factory::stc::position_save()
{
  m_saved_pos = GetCurrentPos();

  if (is_visual() && vi_is_visual())
  {
    m_saved_selection_start = GetSelectionStart();
    m_saved_selection_end   = GetSelectionEnd();
  }
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

#define BIGWORD(DIRECTION)                                                     \
  int c      = GetCharAt(GetCurrentPos());                                     \
  int offset = strncmp((#DIRECTION), "Left", 4) == 0 ? -1 : 0;                 \
  while (isspace(c) && GetCurrentPos() > 0 &&                                  \
         GetCurrentPos() < GetTextLength())                                    \
  {                                                                            \
    Char##DIRECTION();                                                         \
    c = GetCharAt(GetCurrentPos() + offset);                                   \
  }                                                                            \
  while (!isspace(c) && GetCurrentPos() > 0 &&                                 \
         GetCurrentPos() < GetTextLength())                                    \
  {                                                                            \
    Char##DIRECTION();                                                         \
    c = GetCharAt(GetCurrentPos() + offset);                                   \
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
