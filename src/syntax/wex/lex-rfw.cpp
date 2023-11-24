////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw.cpp
// Purpose:   Implementation of lmRFW
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include "lex-rfw.h"

const CharacterSet setWord(CharacterSet::setAlphaNum, "._+-]*");
const CharacterSet setWordStart(CharacterSet::setAlpha, ":_[*");
int                numBase = 0;

// We do not use lexicalClasses (see LexCPP), this would require override
// NameOfStyle, and using NameOfStyle in stc as well. Of course possible, but
// for what purpose?
Scintilla::lex_rfw::lex_rfw()
  : DefaultLexer(name(), language())
  , m_sub_styles(style_subable, 0x80, 0x40, 0)
  , m_section_keywords(
      {{"Setting", SECTION_SETTING},
       {"Variable", SECTION_VARIABLE},
       {"Test Case", SECTION_TESTCASE},
       {"Task", SECTION_TASK},
       {"Keyword", SECTION_KEYWORD},
       {"Comment", SECTION_COMMENT}})
{
  m_cmd_delimiter.Set("| || |& & && ; ;; ( ) { }");
}

void Scintilla::lex_rfw::init()
{
  /* The recommended header format is *** Settings ***,
     but the header is case-insensitive, surrounding spaces are
     optional, and the number of asterisk characters can vary as long
     as there is one asterisk in the beginning.
     In addition to using the plural format, also singular variants
     like Setting and Test Case are accepted.
     In other words, also *setting would be recognized as a section
     header.
     And it is an error to have both test cases and tasks in the same file.
     -> our compare is not yet case insensitive
    */
  m_regex_section_begin = std::make_unique<wex::regex_part>("\\*+ *");
  m_regex_section_end   = std::make_unique<wex::regex_part>("s? *\\**");

  m_quote       = std::make_unique<quote>(*m_accessor);
  m_quote_stack = std::make_unique<quote_stack>(*m_accessor);

  m_section.reset();
}

void Scintilla::lex_rfw::keywords_update()
{
  for (int i = 0; i < m_keywords2.Length(); i++)
  {
    std::string keyword(m_keywords2.WordAt(i));
    std::replace(keyword.begin(), keyword.end(), '_', ' ');

    if (keyword != "EX")
    {
      m_spaced_keywords.emplace_back(keyword);
    }
    else
    {
      m_visual_mode = false;
    }
  }
}

void Scintilla::lex_rfw::parse_keyword(
  StyleContext& sc,
  int           cmd_state,
  int&          cmd_state_new)
{
  WordList rfwStruct;
  rfwStruct.Set("");

  WordList rfwStruct_in;
  rfwStruct_in.Set(":FOR FOR");

  // "." never used in RFW variable names but used in file names
  if (!setWord.Contains(sc.ch))
  {
    char s[500];
    sc.GetCurrent(s, sizeof(s));

    char s2[2];
    snprintf(s2, sizeof(s2), "%c", sc.ch);

    // allow keywords ending in a whitespace or command delimiter
    const bool keywordEnds = IsASpace(sc.ch) || m_cmd_delimiter.InList(s2);

    // 'IN' may be construct keywords
    if (cmd_state == RFW_CMD_WORD && cmd_state_new != RFW_CMD_SKW_PARTIAL)
    {
      if (strcmp(s, "IN") == 0 && keywordEnds)
        cmd_state_new = RFW_CMD_BODY;
      else
        sc.ChangeState(SCE_SH_IDENTIFIER);

      sc.SetState(SCE_SH_DEFAULT);
      return;
    }

    // detect rfw construct keywords
    if (rfwStruct.InList(s))
    {
      if (cmd_state == RFW_CMD_START && keywordEnds)
        cmd_state_new = RFW_CMD_START;
      else if (cmd_state_new != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // ':FOR' needs 'IN' to be highlighted later
    else if (rfwStruct_in.InList(s))
    {
      if (cmd_state == RFW_CMD_START && keywordEnds)
        cmd_state_new = RFW_CMD_WORD;
      else if (cmd_state_new != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // disambiguate option items and file test operators
    else if (s[0] == '-')
    {
      if (cmd_state != RFW_CMD_TEST && cmd_state_new != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // disambiguate keywords and identifiers
    else if (
      cmd_state != RFW_CMD_START || !(m_keywords1.InList(s) && keywordEnds))
    {
      if (cmd_state_new != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }

    if (cmd_state_new != RFW_CMD_SKW_PARTIAL)
      sc.SetState(SCE_SH_DEFAULT);
  }
}

bool Scintilla::lex_rfw::section_keywords_detect(
  const std::string& word,
  StyleContext&      sc,
  int&               cmd_state_new)
{
  return std::any_of(
    m_section_keywords.begin(),
    m_section_keywords.end(),
    [&sc, &word, &cmd_state_new, this](const auto& i)
    {
      if (std::equal(word.begin(), word.end(), i.first.begin()))
      {
        if (word.size() == i.first.size())
        {
          section_start(i, sc, cmd_state_new);
          return true;
        }
        else
        {
          cmd_state_new = RFW_CMD_SKW_PARTIAL;
        }
      }

      return false;
    });
}

void Scintilla::lex_rfw::section_start(
  const section_keyword_t& section,
  StyleContext&            sc,
  int&                     cmd_state_new)
{
  do
  {
    sc.Forward();
  } while (m_regex_section_end->match(sc.ch) == wex::regex_part::match_t::FULL);

  sc.SetState(SCE_SH_WORD);
  m_section.start(section.second);

  cmd_state_new = RFW_CMD_START;

  if (m_section.id() == SECTION_COMMENT)
  {
    sc.SetState(SCE_SH_COMMENTLINE);
  }
}

bool Scintilla::lex_rfw::spaced_keywords_detect(
  const std::string& word,
  StyleContext&      sc,
  int&               cmd_state_new)
{
  return std::any_of(
    m_spaced_keywords.begin(),
    m_spaced_keywords.end(),
    [&sc, &word, &cmd_state_new](const auto& i)
    {
      if (std::equal(word.begin(), word.end(), i.begin()))
      {
        if (word.size() == i.size())
        {
          sc.Forward();
          sc.SetState(SCE_SH_WORD);
          cmd_state_new = RFW_CMD_START;
          return true;
        }
        else
        {
          cmd_state_new = RFW_CMD_SKW_PARTIAL;
        }
      }
      return false;
    });
}

void Scintilla::lex_rfw::state_check(
  StyleContext& sc,
  int           cmd_state,
  int&          cmd_state_new)
{
  // Determine if the current state should terminate.
  switch (sc.state)
  {
    case SCE_SH_OPERATOR:
      sc.SetState(SCE_SH_DEFAULT);
      if (cmd_state == RFW_CMD_DELIM) // if command delimiter, start new
                                      // command
        cmd_state_new = RFW_CMD_START;
      else if (sc.chPrev == '\\') // propagate command state if line continued
        cmd_state_new = cmd_state;
      break;

    case SCE_SH_TESTCASE:
      if (!setWord.Contains(sc.ch) || sc.chNext == '*')
      {
        cmd_state_new = RFW_CMD_START;
        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_WORD:
    case SCE_SH_WORD2:
      parse_keyword(sc, cmd_state, cmd_state_new);
      break;

    case SCE_SH_IDENTIFIER:
      if (sc.chPrev == '\\')
      { // for escaped chars
        sc.ForwardSetState(SCE_SH_DEFAULT);
      }
      else if (!setWord.Contains(sc.ch))
      {
        sc.SetState(SCE_SH_DEFAULT);
      }
      else if (cmd_state == RFW_CMD_ARITH && !setWordStart.Contains(sc.ch))
      {
        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_NUMBER:
      if (auto digit = lex_rfw_access(*m_accessor).translate_digit(sc.ch);
          numBase == RFW_BASE_DECIMAL)
      {
        if (sc.ch == '#')
        {
          char s[10];
          sc.GetCurrent(s, sizeof(s));
          numBase = lex_rfw_access(*m_accessor).number_base(s);
          if (numBase != RFW_BASE_ERROR)
            break;
        }
        else if (IsADigit(sc.ch))
          break;
      }
      else if (numBase == RFW_BASE_HEX)
      {
        if (IsADigit(sc.ch, 16))
          break;
      }
      else if (numBase == RFW_BASE_ERROR)
      {
        if (digit <= 9)
          break;
      }
      else
      { // DD#DDDD number style handling
        if (digit != RFW_BASE_ERROR)
        {
          if (numBase <= 36)
          {
            // case-insensitive if base<=36
            if (digit >= 36)
              digit -= 26;
          }
          if (digit < numBase)
            break;
          if (digit <= 9)
          {
            numBase = RFW_BASE_ERROR;
            break;
          }
        }
      }
      // fallthrough when number is at an end or error
      if (numBase == RFW_BASE_ERROR)
      {
        sc.ChangeState(SCE_SH_ERROR);
      }
      sc.SetState(SCE_SH_DEFAULT);
      break;

    case SCE_SH_COMMENTLINE:
      if (m_visual_mode && sc.atLineEnd && sc.chPrev != '\\')
      {
        if (m_section.id() == SECTION_COMMENT)
        {
          if (sc.chPrev != '\n' && sc.GetRelative(-2) != '\n')
            return;
        }

        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_SCALAR: // variable names
      if (const CharacterSet setParam(CharacterSet::setAlphaNum, "$@_");
          !setParam.Contains(sc.ch))
      {
        if (sc.LengthCurrent() == 1)
        {
          // section variable: $(, $_ etc.
          sc.ForwardSetState(SCE_SH_DEFAULT);
        }
        else
        {
          sc.SetState(SCE_SH_DEFAULT);
        }
      }
      break;

    case SCE_SH_STRING: // delimited styles, can nest
    case SCE_SH_BACKTICKS:
      if (sc.ch == '\\' && m_quote_stack->up() != '\\')
      {
        if (m_quote_stack->style() != RFW_DELIM_LITERAL)
          sc.Forward();
      }
      else if (sc.ch == m_quote_stack->down())
      {
        m_quote_stack->decrease();

        if (m_quote_stack->count() == 0)
        {
          if (m_quote_stack->depth() > 0)
          {
            m_quote_stack->pop();
          }
          else
            sc.ForwardSetState(SCE_SH_DEFAULT);
        }
      }
      else if (sc.ch == m_quote_stack->up())
      {
        m_quote_stack->increase();
      }
      else
      {
        if (
          m_quote_stack->style() == RFW_DELIM_STRING ||
          m_quote_stack->style() == RFW_DELIM_LSTRING)
        { // do nesting for "string", $"locale-string"
          if (sc.ch == '`')
          {
            m_quote_stack->push(sc.ch, RFW_DELIM_BACKTICK);
          }
          else if (sc.ch == '$' && sc.chNext == '(')
          {
            sc.Forward();
            m_quote_stack->push(sc.ch, RFW_DELIM_COMMAND);
          }
        }
        else if (
          m_quote_stack->style() == RFW_DELIM_COMMAND ||
          m_quote_stack->style() == RFW_DELIM_BACKTICK)
        { // do nesting for $(command), `command`
          if (sc.ch == '\'')
          {
            m_quote_stack->push(sc.ch, RFW_DELIM_LITERAL);
          }
          else if (sc.ch == '\"' && !m_options.vi_script())
          {
            m_quote_stack->push(sc.ch, RFW_DELIM_STRING);
          }
          else if (sc.ch == '`')
          {
            m_quote_stack->push(sc.ch, RFW_DELIM_BACKTICK);
          }
          else if (sc.ch == '$')
          {
            if (sc.chNext == '\'')
            {
              sc.Forward();
              m_quote_stack->push(sc.ch, RFW_DELIM_CSTRING);
            }
            else if (sc.chNext == '\"' && !m_options.vi_script())
            {
              sc.Forward();
              m_quote_stack->push(sc.ch, RFW_DELIM_LSTRING);
            }
            else if (sc.chNext == '(')
            {
              sc.Forward();
              m_quote_stack->push(sc.ch, RFW_DELIM_COMMAND);
            }
          }
        }
      }
      break;

    case SCE_SH_PARAM: // ${parameter}
      if (sc.ch == '\\' && m_quote->up() != '\\')
      {
        sc.Forward();
      }
      else if (sc.ch == m_quote->down())
      {
        m_quote->decrease();

        if (m_quote->count() == 0)
        {
          sc.ForwardSetState(SCE_SH_DEFAULT);
        }
      }
      else if (sc.ch == m_quote->up())
      {
        m_quote->increase();
      }
      break;

    case SCE_SH_CHARACTER: // singly-quoted strings
      if (sc.ch == m_quote->down())
      {
        m_quote->decrease();
        if (m_quote->count() == 0)
        {
          sc.ForwardSetState(SCE_SH_DEFAULT);
        }
      }
      break;
  }
}

bool Scintilla::lex_rfw::state_check_continue(StyleContext& sc, int& cmd_state)
{
  const CharacterSet setMetaCharacter(CharacterSet::setNone, "|&;()<> \t\r\n");
  const CharacterSet setRFWOperator(
    CharacterSet::setNone,
    "^&%()-+={};>,/<?!.~@");
  const CharacterSet setWordStartTSV(CharacterSet::setAlphaNum, ":_[*");
  // note that [+-] are often parts of identifiers in shell scripts
  const CharacterSet setSingleCharOp(
    CharacterSet::setNone,
    "rwxoRWXOezsfdlpSbctugkTBMACahGLNn");
  int testExprType = 0;

  if (sc.ch == '\\')
  {
    // RFW can escape any non-newline as a literal
    sc.SetState(SCE_SH_IDENTIFIER);
    if (sc.chNext == '\r' || sc.chNext == '\n')
      sc.SetState(SCE_SH_OPERATOR);
  }
  else if (setWordStartTSV.Contains(sc.ch))
  {
    // README: or set to SCE_SH_WORD2
    sc.SetState(cmd_state == RFW_CMD_TESTCASE ? SCE_SH_TESTCASE : SCE_SH_WORD);
  }
  else if (IsADigit(sc.ch))
  {
    sc.SetState(SCE_SH_NUMBER);
    numBase = RFW_BASE_DECIMAL;
    if (sc.ch == '0')
    { // hex,octal
      if (sc.chNext == 'x' || sc.chNext == 'X')
      {
        numBase = RFW_BASE_HEX;
        sc.Forward();
      }
      else if (IsADigit(sc.chNext))
      {
        numBase = RFW_BASE_HEX;
      }
    }
  }
  else if (sc.ch == '#')
  {
    if (
      m_style_prev != SCE_SH_WORD && m_style_prev != SCE_SH_WORD2 &&
      m_style_prev != SCE_SH_IDENTIFIER &&
      (sc.currentPos == 0 || setMetaCharacter.Contains(sc.chPrev)))
    {
      sc.SetState(SCE_SH_COMMENTLINE);
    }
    else
    {
      sc.SetState(SCE_SH_WORD);
    }
    // handle some zsh features within arithmetic expressions only
    if (cmd_state == RFW_CMD_ARITH)
    {
      if (sc.chPrev == '[')
      { // [#8] [##8] output digit setting
        sc.SetState(SCE_SH_WORD);
        if (sc.chNext == '#')
        {
          sc.Forward();
        }
      }
      else if (sc.Match("##^") && IsUpperCase(sc.GetRelative(3)))
      { // ##^A
        sc.SetState(SCE_SH_IDENTIFIER);
        sc.Forward(3);
      }
      else if (sc.chNext == '#' && !IsASpace(sc.GetRelative(2)))
      { // ##            sc.SetState(SCE_SH_IDENTIFIER);
        sc.Forward(2);
      }
      else if (setWordStart.Contains(sc.chNext))
      { // #name
        sc.SetState(SCE_SH_IDENTIFIER);
      }
    }
  }
  else if (sc.ch == '\"' && !m_options.vi_script())
  {
    sc.SetState(SCE_SH_STRING);
    m_quote_stack->start(sc.ch, RFW_DELIM_STRING);
  }
  else if (sc.ch == '`')
  {
    sc.SetState(SCE_SH_BACKTICKS);
    m_quote_stack->start(sc.ch, RFW_DELIM_BACKTICK);
  }
  else if (sc.ch == '$' || sc.ch == '@')
  {
    if (sc.Match("$(("))
    {
      sc.SetState(SCE_SH_OPERATOR); // handle '((' later
      return true;
    }
    sc.SetState(SCE_SH_SCALAR);
    sc.Forward();
    if (sc.ch == '\'')
    {
      sc.ChangeState(SCE_SH_STRING);
      m_quote_stack->start(sc.ch, RFW_DELIM_CSTRING);
    }
    else if (sc.ch == '"')
    {
      sc.ChangeState(SCE_SH_STRING);
      m_quote_stack->start(sc.ch, RFW_DELIM_LSTRING);
    }
    else if (sc.ch == '(')
    {
      sc.ChangeState(SCE_SH_BACKTICKS);
      m_quote_stack->start(sc.ch, RFW_DELIM_COMMAND);
    }
    else if (sc.ch == '`')
    { // $` seen in a configure script, valid?
      sc.ChangeState(SCE_SH_BACKTICKS);
      m_quote_stack->start(sc.ch, RFW_DELIM_BACKTICK);
    }
    else
    {
      return true; // scalar has no delimiter pair
    }
  }
  else if (
    sc.ch == '-' && // one-char file test operators
    setSingleCharOp.Contains(sc.chNext) &&
    !setWord.Contains(sc.GetRelative(2)) && IsASpace(sc.chPrev))
  {
    sc.SetState(SCE_SH_WORD);
    sc.Forward();
  }
  else if (setRFWOperator.Contains(sc.ch))
  {
    sc.SetState(SCE_SH_OPERATOR);
    // globs have no whitespace, do not appear in arithmetic expressions
    if (cmd_state != RFW_CMD_ARITH && sc.ch == '(' && sc.chNext != '(')
    {
      int i = lex_rfw_access(*m_accessor).glob_scan(sc);
      if (i > 1)
      {
        sc.SetState(SCE_SH_IDENTIFIER);
        sc.Forward(i);
        return true;
      }
    }
    // handle opening delimiters for test/arithmetic expressions - ((,[[,[
    if (cmd_state == RFW_CMD_START || cmd_state == RFW_CMD_BODY)
    {
      if (sc.Match('(', '('))
      {
        cmd_state = RFW_CMD_ARITH;
        sc.Forward();
      }
      else if (sc.Match('[', '[') && IsASpace(sc.GetRelative(2)))
      {
        cmd_state    = RFW_CMD_TEST;
        testExprType = 1;
        sc.Forward();
      }
      else if (sc.ch == '[' && IsASpace(sc.chNext))
      {
        cmd_state    = RFW_CMD_TEST;
        testExprType = 2;
      }
    }
    // section state -- for ((x;y;z)) in ... looping
    if (cmd_state == RFW_CMD_WORD && sc.Match('(', '('))
    {
      cmd_state = RFW_CMD_ARITH;
      sc.Forward();
      return true;
    }
    // handle command delimiters in command START|BODY|WORD state, also TEST
    // if 'test'
    if (
      cmd_state == RFW_CMD_START || cmd_state == RFW_CMD_BODY ||
      cmd_state == RFW_CMD_WORD ||
      (cmd_state == RFW_CMD_TEST && testExprType == 0))
    {
      char s[10];
      bool isCmdDelim = false;
      s[0]            = static_cast<char>(sc.ch);

      if (setRFWOperator.Contains(sc.chNext))
      {
        s[1]       = static_cast<char>(sc.chNext);
        s[2]       = '\0';
        isCmdDelim = m_cmd_delimiter.InList(s);
        if (isCmdDelim)
          sc.Forward();
      }
      if (!isCmdDelim)
      {
        s[1]       = '\0';
        isCmdDelim = m_cmd_delimiter.InList(s);
      }
      if (isCmdDelim)
      {
        cmd_state = RFW_CMD_DELIM;
        return true;
      }
    }
    // handle closing delimiters for test/arithmetic expressions - )),]],]
    if (cmd_state == RFW_CMD_ARITH && sc.Match(')', ')'))
    {
      cmd_state = RFW_CMD_BODY;
      sc.Forward();
    }
    else if (cmd_state == RFW_CMD_TEST && IsASpace(sc.chPrev))
    {
      if (sc.Match(']', ']') && testExprType == 1)
      {
        sc.Forward();
        cmd_state = RFW_CMD_BODY;
      }
      else if (sc.ch == ']' && testExprType == 2)
      {
        cmd_state = RFW_CMD_BODY;
      }
    }
  }

  return false;
}

void SCI_METHOD Scintilla::lex_rfw::Fold(
  Sci_PositionU startPos,
  Sci_Position  length,
  int,
  IDocument* pAccess)
{
  m_accessor = std::make_unique<LexAccessor>(pAccess);

  Sci_PositionU endPos      = startPos + length;
  Sci_Position  lineCurrent = m_accessor->GetLine(startPos);

  int levelPrev    = m_accessor->LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK,
      levelCurrent = levelPrev, visibleChars = 0;

  char chNext = (*m_accessor)[startPos];

  for (Sci_PositionU i = startPos; i < endPos; i++)
  {
    char ch = chNext;
    chNext  = m_accessor->SafeGetCharAt(i + 1);

    const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

    lex_rfw_access rfw(*m_accessor, lineCurrent);

    // Comment folding
    if (m_options.fold_comment() && atEOL && rfw.is_comment_line())
    {
      if (!rfw.is_comment_line(-1) && rfw.is_comment_line(1))
        levelCurrent++;
      else if (rfw.is_comment_line(-1) && !rfw.is_comment_line(1))
        levelCurrent--;
    }

    // Pipe folding
    if (m_options.fold_pipes() && atEOL && rfw.is_pipe_line())
    {
      if (!rfw.is_pipe_line(-1) && rfw.is_pipe_line(1))
        levelCurrent++;
      else if (rfw.is_pipe_line(-1) && !rfw.is_pipe_line(1))
        levelCurrent--;
    }

    // Tab folding
    if (m_options.fold_tabs() && atEOL && rfw.is_tab_line())
    {
      if (!rfw.is_tab_line(-1) && rfw.is_tab_line(1))
        levelCurrent++;
      else if (rfw.is_tab_line(-1) && !rfw.is_tab_line(1))
        levelCurrent--;
    }

    if (atEOL)
    {
      int lev = levelPrev;
      if (visibleChars == 0 && m_options.fold_compact())
        lev |= SC_FOLDLEVELWHITEFLAG;
      if ((levelCurrent > levelPrev) && (visibleChars > 0))
        lev |= SC_FOLDLEVELHEADERFLAG;
      if (lev != m_accessor->LevelAt(lineCurrent))
      {
        m_accessor->SetLevel(lineCurrent, lev);
      }
      lineCurrent++;
      levelPrev    = levelCurrent;
      visibleChars = 0;
    }

    if (!isspacechar(ch))
      visibleChars++;
  }

  // Fill in the real level of the next line, keeping the current flags as
  // they will be filled in later
  int flagsNext = m_accessor->LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
  m_accessor->SetLevel(lineCurrent, levelPrev | flagsNext);
}

void SCI_METHOD Scintilla::lex_rfw::Lex(
  Sci_PositionU startPos,
  Sci_Position  length,
  int           initStyle,
  IDocument*    pAccess)
{
  m_accessor = std::make_unique<LexAccessor>(pAccess);

  init();

  bool pipes     = false;
  int  cmd_state = RFW_CMD_START;

  Sci_PositionU endPos = startPos + length;
  Sci_Position  ln     = lex_rfw_access(*m_accessor).init(startPos);
  initStyle            = SCE_SH_DEFAULT;

  std::string words;

  StyleContext sc(startPos, endPos - startPos, initStyle, *m_accessor);

  for (; sc.More(); sc.Forward())
  {
    if (sc.ch == '|' && sc.atLineStart)
    {
      pipes = true;
    }

    // handle line continuation, updates per-line stored state
    if (sc.atLineStart)
    {
      ln = m_accessor->GetLine(sc.currentPos);
      if (
        sc.state == SCE_SH_STRING || sc.state == SCE_SH_BACKTICKS ||
        sc.state == SCE_SH_CHARACTER || sc.state == SCE_SH_COMMENTLINE ||
        sc.state == SCE_SH_PARAM)
      {
        // force backtrack while retaining cmd_state
        m_accessor->SetLineState(ln, RFW_CMD_BODY);
      }
      else
      {
        if (ln > 0)
        {
          if (
            (sc.GetRelative(-3) == '\\' && sc.GetRelative(-2) == '\r' &&
             sc.chPrev == '\n') ||
            sc.GetRelative(-2) == '\\')
          { // handle '\' line continuation
            // retain last line's state
          }
          else
            cmd_state = RFW_CMD_START;
        }
        m_accessor->SetLineState(ln, cmd_state);
      }
    }

    // controls change of cmd_state at the end of a non-whitespace element
    // states BODY|TEST|ARITH persist until the end of a command segment
    // state WORD persist, but ends with 'in' or 'do' construct keywords
    int cmd_state_new = RFW_CMD_BODY;
    if (
      cmd_state == RFW_CMD_TEST || cmd_state == RFW_CMD_ARITH ||
      cmd_state == RFW_CMD_WORD)
      cmd_state_new = cmd_state;

    m_style_prev = sc.state;

    words.append(1, sc.ch);

    if (!spaced_keywords_detect(words, sc, cmd_state_new))
    {
      if (
        m_regex_section_begin->match_type() ==
          wex::regex_part::match_t::HISTORY ||
        m_regex_section_begin->match(sc.ch) >= wex::regex_part::match_t::PART)
      {
        if (
          section_keywords_detect(
            words.substr(m_regex_section_begin->text().size()),
            sc,
            cmd_state_new) &&
          m_section.is_case())
        {
          // remove the other testcase like section as there can be only one of
          // them
          const auto other_section =
            (m_section.id() == SECTION_TESTCASE ? SECTION_TASK :
                                                  SECTION_TESTCASE);

          if (const auto& removed(std::remove_if(
                m_section_keywords.begin(),
                m_section_keywords.end(),
                [&](const auto& it)
                {
                  return it.second == other_section;
                }));
              removed != m_section_keywords.end())
          {
            m_section_keywords.erase(removed);
          }
        }
      }

      if (cmd_state_new != RFW_CMD_SKW_PARTIAL)
      {
        words.clear();
        m_regex_section_end->reset();
        m_regex_section_begin->reset();
      }
    }

    if (
      (sc.chPrev == '|' && sc.ch == ' ') && cmd_state != RFW_CMD_TESTCASE &&
      cmd_state != RFW_CMD_WORD)
    {
      cmd_state = RFW_CMD_START;
    }

    state_check(sc, cmd_state, cmd_state_new);

    // update cmd_state about the current command segment
    if (m_style_prev != SCE_SH_DEFAULT && sc.state == SCE_SH_DEFAULT)
    {
      cmd_state = cmd_state_new;
    }
    else if (pipes && sc.ch == '|' && m_section.is_case())
    {
      cmd_state = (sc.atLineStart ? RFW_CMD_TESTCASE : RFW_CMD_START);
    }
    else if (
      m_visual_mode && !pipes && sc.ch != '#' && !isspace(sc.ch) &&
      sc.atLineStart)
    {
      if (m_section.is_case())
      {
        cmd_state = RFW_CMD_TESTCASE;
      }
    }

    if (sc.state == SCE_SH_DEFAULT)
    {
      if (state_check_continue(sc, cmd_state))
      {
        continue;
      }
    }
  }

  sc.Complete();
}

void SCI_METHOD Scintilla::lex_rfw::Release()
{
  delete this;
}

Sci_Position SCI_METHOD Scintilla::lex_rfw::WordListSet(int n, const char* wl)
{
  Sci_Position firstModification = -1;
  WordList*    wordList          = nullptr;

  switch (n)
  {
    case 0:
      wordList = &m_keywords1;
      break;

    case 1:
      wordList = &m_keywords2;
      break;

    default:
      return firstModification;
  }

  if (wordList != nullptr)
  {
    WordList wlNew;
    wlNew.Set(wl);

    if (*wordList != wlNew)
    {
      wordList->Set(wl);
      firstModification = 0;

      if (n == 1)
      {
        keywords_update();
      }
    }
  }

  return firstModification;
}

LexerModule lmRFW(
  lex_rfw::language(),
  lex_rfw::get,
  lex_rfw::name(),
  option_set_rfw::keywords());
