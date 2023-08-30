////////////////////////////////////////////////////////////////////////////////
// Name:      lex-lilypond.cpp
// Purpose:   Implementation of Scintilla::lex_lillypond
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "lex-lilypond.h"
#include "lex-lilypond-util.h"

#define STATE_ERROR()              \
  styler.ColourTo(i, SCE_L_ERROR); \
  state = mode_to_state(mode);     \
  ch    = styler.SafeGetCharAt(i); \
  if (ch == '\r' || ch == '\n')    \
    set_modes(styler.GetLine(i), mode);

lex_lilypond::lex_lilypond()
  : DefaultLexer(name(), language())
{
}

void lex_lilypond::fold_dec(int& lev, fold_save& save) const
{
  while (save.m_level > 0 && save.m_open_begins[save.m_level] == 0)
    --save.m_level;

  if (lev < 0)
    lev = save.to_int();

  if (save.m_open_begins[save.m_level] > 0)
    --save.m_open_begins[save.m_level];
}

void lex_lilypond::fold_inc(int& lev, fold_save& save, bool& need) const
{
  if (lev < 0)
    lev = save.to_int();

  ++save.m_open_begins[save.m_level];
  need = true;
}

int lex_lilypond::mode_to_state(int mode) const
{
  switch (mode)
  {
    case 1:
      return SCE_L_MATH;
    case 2:
      return SCE_L_MATH2;
    default:
      return SCE_L_DEFAULT;
  }
}

// Change folding state while processing a line
// Return the level before the first relevant command
void SCI_METHOD lex_lilypond::Fold(
  Sci_PositionU startPos,
  Sci_Position  length,
  int,
  IDocument* pAccess)
{
  const std::vector<std::string> structWords{
    "part",
    "chapter",
    "section",
    "subsection",
    "subsubsection",
    "paragraph",
    "subparagraph"};

  LexAccessor   styler(pAccess);
  Sci_PositionU endPos  = startPos + length;
  Sci_Position  curLine = styler.GetLine(startPos);
  fold_save     save;

  get_save(curLine - 1, save);

  do
  {
    char         ch, buf[16];
    Sci_Position i, j;
    int          lev      = -1;
    bool         needFold = false;
    for (i = static_cast<Sci_Position>(startPos);
         i < static_cast<Sci_Position>(endPos);
         ++i)
    {
      ch = styler.SafeGetCharAt(i);
      if (ch == '\r' || ch == '\n')
        break;
      if (ch == '{')
      {
        fold_inc(lev, save, needFold);
      }
      else if (ch == '}')
      {
        fold_dec(lev, save);
      }
      else if (ch != '\\' || styler.StyleAt(i) != SCE_L_COMMAND)
        continue;

      for (j = 0; j < 15 && i + 1 < static_cast<Sci_Position>(endPos); ++j, ++i)
      {
        buf[j] = styler.SafeGetCharAt(i + 1);
        if (!lilypond::is_letter(buf[j]))
          break;
      }
      buf[j] = '\0';
      if (strcmp(buf, "begin") == 0)
      {
        fold_inc(lev, save, needFold);
      }
      else if (strcmp(buf, "end") == 0)
      {
        fold_dec(lev, save);
      }
      else
      {
        for (j = 0; j < structWords.size(); ++j)
          if (strcmp(buf, structWords[j].c_str()) == 0)
            break;
        if (j >= structWords.size())
          continue;
        save.m_level = j; // level before the command
        for (j = save.m_level + 1; j < save.m_open_begins.size(); ++j)
        {
          save.m_open_begins[save.m_level] += save.m_open_begins[j];
          save.m_open_begins[j] = 0;
        }
        if (lev < 0)
          lev = save.to_int();
        ++save.m_level; // level after the command
        needFold = true;
      }
    }
    if (lev < 0)
      lev = save.to_int();
    if (needFold)
      lev |= SC_FOLDLEVELHEADERFLAG;
    styler.SetLevel(curLine, lev);
    set_saves(curLine, save);
    ++curLine;
    startPos = styler.LineStart(curLine);

    if (static_cast<Sci_Position>(startPos) == styler.Length())
    {
      lev = save.to_int();
      styler.SetLevel(curLine, lev);
      set_saves(curLine, save);
      resize_saves(curLine);
    }
  } while (startPos < endPos);

  styler.Flush();
}

// There are cases not handled correctly, like $abcd\textrm{what is $x+y$}z+w$.
// But I think it's already good enough.
void SCI_METHOD lex_lilypond::Lex(
  Sci_PositionU startPos,
  Sci_Position  length,
  int           initStyle,
  IDocument*    pAccess)
{
  // startPos is assumed to be the first character of a line
  LexAccessor styler(pAccess);
  styler.StartAt(startPos);
  int mode  = get_mode(styler.GetLine(startPos) - 1);
  int state = initStyle;
  if (
    state == SCE_L_ERROR || state == SCE_L_SHORTCMD ||
    state == SCE_L_SPECIAL) // should not happen
    state = mode_to_state(mode);

  char chNext          = styler.SafeGetCharAt(startPos);
  char chVerbatimDelim = '\0';
  styler.StartSegment(startPos);
  Sci_Position lengthDoc = startPos + length;

  for (Sci_Position i = startPos; i < lengthDoc; i++)
  {
    char ch = chNext;
    chNext  = styler.SafeGetCharAt(i + 1);

    if (styler.IsLeadByte(ch))
    {
      i++;
      chNext = styler.SafeGetCharAt(i + 1);
      continue;
    }

    if (ch == '\r' || ch == '\n')
      set_modes(styler.GetLine(i), mode);

    switch (state)
    {
      case SCE_L_DEFAULT:
        switch (ch)
        {
          case '\\':
            styler.ColourTo(i - 1, state);
            if (lilypond::is_letter(chNext))
            {
              state = SCE_L_COMMAND;
            }
            else if (lilypond(styler).is_special(chNext, i))
            {
            }
            else if (chNext == '\r' || chNext == '\n')
            {
              styler.ColourTo(i, SCE_L_ERROR);
            }
            else if (IsASCII(chNext))
            {
              styler.ColourTo(i + 1, SCE_L_SHORTCMD);
              if (chNext == '(')
              {
                mode  = 1;
                state = SCE_L_MATH;
              }
              else if (chNext == '[')
              {
                mode  = 2;
                state = SCE_L_MATH2;
              }
              i++;
              chNext = styler.SafeGetCharAt(i + 1);
            }
            break;
          case '$':
            styler.ColourTo(i - 1, state);
            if (chNext == '$')
            {
              styler.ColourTo(i + 1, SCE_L_SHORTCMD);
              mode  = 2;
              state = SCE_L_MATH2;
              i++;
              chNext = styler.SafeGetCharAt(i + 1);
            }
            else
            {
              styler.ColourTo(i, SCE_L_SHORTCMD);
              mode  = 1;
              state = SCE_L_MATH;
            }
            break;
          case '%':
            styler.ColourTo(i - 1, state);
            state = SCE_L_COMMENT;
            break;
        }
        break;
      // These 3 will never be reached.
      case SCE_L_ERROR:
      case SCE_L_SPECIAL:
      case SCE_L_SHORTCMD:
        break;
      case SCE_L_COMMAND:
        if (!lilypond::is_letter(chNext))
        {
          styler.ColourTo(i, state);
          lilypond lp(styler);
          if (lp.next_not_blank_is(i + 1, '['))
          {
            state = SCE_L_CMDOPT;
          }
          else if (lp.last_word_is(i, "\\begin"))
          {
            state = SCE_L_TAG;
          }
          else if (lp.last_word_is(i, "\\end"))
          {
            state = SCE_L_TAG2;
          }
          else if (
            lp.last_word_is(i, "\\verb") && chNext != '*' && chNext != ' ')
          {
            chVerbatimDelim = chNext;
            state           = SCE_L_VERBATIM;
          }
          else
          {
            state = mode_to_state(mode);
          }
        }
        break;
      case SCE_L_CMDOPT:
        if (ch == ']')
        {
          styler.ColourTo(i, state);
          state = mode_to_state(mode);
        }
        break;
      case SCE_L_TAG:
        if (lilypond lp(styler); lp.is_tag_valid(i, lengthDoc))
        {
          styler.ColourTo(i, state);
          state = mode_to_state(mode);
          if (lp.last_word_is(i, "{verbatim}"))
          {
            state = SCE_L_VERBATIM;
          }
          else if (lp.last_word_is(i, "{lstlisting}"))
          {
            state = SCE_L_VERBATIM;
          }
          else if (lp.last_word_is(i, "{comment}"))
          {
            state = SCE_L_COMMENT2;
          }
          else if (lp.last_word_is(i, "{math}") && mode == 0)
          {
            mode  = 1;
            state = SCE_L_MATH;
          }
          else if (lp.last_word_is_match_env(i) && mode == 0)
          {
            mode  = 2;
            state = SCE_L_MATH2;
          }
        }
        else
        {
          STATE_ERROR();
        }
        chNext = styler.SafeGetCharAt(i + 1);
        break;
      case SCE_L_TAG2:
        if (lilypond(styler).is_tag_valid(i, lengthDoc))
        {
          styler.ColourTo(i, state);
          state = mode_to_state(mode);
        }
        else
        {
          STATE_ERROR();
        }
        chNext = styler.SafeGetCharAt(i + 1);
        break;
      case SCE_L_MATH:
        switch (ch)
        {
          case '\\':
            styler.ColourTo(i - 1, state);
            if (lilypond::is_letter(chNext))
            {
              if (lilypond lp(styler);
                  lp.last_word_check(i, "\\end", {"math"}, lengthDoc, state))
              {
                mode = 0;
              }
              state = SCE_L_COMMAND;
            }
            else if (lilypond(styler).is_special(chNext, i))
            {
            }
            else if (chNext == '\r' || chNext == '\n')
            {
              styler.ColourTo(i, SCE_L_ERROR);
            }
            else if (IsASCII(chNext))
            {
              if (chNext == ')')
              {
                mode  = 0;
                state = SCE_L_DEFAULT;
              }
              styler.ColourTo(i + 1, SCE_L_SHORTCMD);
              i++;
              chNext = styler.SafeGetCharAt(i + 1);
            }
            break;
          case '$':
            styler.ColourTo(i - 1, state);
            styler.ColourTo(i, SCE_L_SHORTCMD);
            mode  = 0;
            state = SCE_L_DEFAULT;
            break;
          case '%':
            styler.ColourTo(i - 1, state);
            state = SCE_L_COMMENT;
            break;
        }
        break;
      case SCE_L_MATH2:
        switch (ch)
        {
          case '\\':
            styler.ColourTo(i - 1, state);
            if (lilypond::is_letter(chNext))
            {
              if (lilypond lp(styler);
                  lp.last_word_check(i, "\\end", {}, lengthDoc, state))
              {
                mode = 0;
              }
              state = SCE_L_COMMAND;
            }
            else if (lilypond(styler).is_special(chNext, i))
            {
            }
            else if (chNext == '\r' || chNext == '\n')
            {
              styler.ColourTo(i, SCE_L_ERROR);
            }
            else if (IsASCII(chNext))
            {
              if (chNext == ']')
              {
                mode  = 0;
                state = SCE_L_DEFAULT;
              }
              styler.ColourTo(i + 1, SCE_L_SHORTCMD);
              i++;
              chNext = styler.SafeGetCharAt(i + 1);
            }
            break;
          case '$':
            styler.ColourTo(i - 1, state);
            if (chNext == '$')
            {
              styler.ColourTo(i + 1, SCE_L_SHORTCMD);
              i++;
              chNext = styler.SafeGetCharAt(i + 1);
              mode   = 0;
              state  = SCE_L_DEFAULT;
            }
            else
            { // This may not be an error, e.g.
              // \begin{equation}\text{$a$}\end{equation}
              styler.ColourTo(i, SCE_L_SHORTCMD);
            }
            break;
          case '%':
            styler.ColourTo(i - 1, state);
            state = SCE_L_COMMENT;
            break;
        }
        break;
      case SCE_L_COMMENT:
        if (ch == '\r' || ch == '\n')
        {
          styler.ColourTo(i - 1, state);
          state = mode_to_state(mode);
        }
        break;
      case SCE_L_COMMENT2:
        if (ch == '\\')
        {
          lilypond(styler)
            .last_word_check(i, "\\end", {"comment"}, lengthDoc, state);
        }
        break;
      case SCE_L_VERBATIM:
        if (ch == '\\')
        {
          lilypond(styler).last_word_check(
            i,
            "\\end",
            {"verbatim", "lstlisting"},
            lengthDoc,
            state);
        }
        else if (chNext == chVerbatimDelim)
        {
          styler.ColourTo(i + 1, state);
          state           = mode_to_state(mode);
          chVerbatimDelim = '\0';
          i++;
          chNext = styler.SafeGetCharAt(i + 1);
        }
        else if (chVerbatimDelim != '\0' && (ch == '\n' || ch == '\r'))
        {
          styler.ColourTo(i, SCE_L_ERROR);
          state           = mode_to_state(mode);
          chVerbatimDelim = '\0';
        }
        break;
    }
  }

  if (lengthDoc == styler.Length())
  {
    resize_modes(styler.GetLine(lengthDoc - 1));
  }

  styler.ColourTo(lengthDoc - 1, state);
  styler.Flush();
}

static const char* const emptyWordListDesc[] = {0};

LexerModule lmLilyPond(
  lex_lilypond::language(),
  lex_lilypond::get,
  lex_lilypond::name(),
  emptyWordListDesc);
