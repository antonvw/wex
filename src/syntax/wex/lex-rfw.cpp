////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw.cpp
// Purpose:   Implementation of lmRFW
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace Scintilla;

#include "lex-rfw.h"

#define ADD_SECTION(REGEX, NAME) {regex_part(REGEX, std::regex::icase), NAME},

const CharacterSet setWord(CharacterSet::setAlphaNum, "._+-]*");
const CharacterSet setWordStart(CharacterSet::setAlpha, ":_[*");
int                numBase = 0;

// We do not use lexicalClasses (see LexCPP), this would require override
// NameOfStyle, and using NameOfStyle in stc as well. Of course possible, but
// for what purpose?
wex::lex_rfw::lex_rfw()
  : DefaultLexer(name(), language())
  , m_sub_styles(style_subable, 0x80, 0x40, 0)
  /* The recommended header format is *** Settings ***,
     but the header is case-insensitive, surrounding spaces are
     optional, and the number of asterisk characters can vary as long
     as there is one asterisk in the beginning.
     In addition to using the plural format, also singular variants
     like Setting and Test Case are accepted.
     In other words, also *setting would be recognized as a section
     header.
     -> we require 3 *, regex_part issue
    */
  , m_section_keywords(
      // clang-format off
      {ADD_SECTION("\\*+ *Settings? *\\*\\*\\*", SECTION_SETTING)
       ADD_SECTION("\\*+ *Variables? *\\*\\*\\*", SECTION_VARIABLE)
       ADD_SECTION("\\*+ *Test *Cases? *\\*\\*\\*", SECTION_TESTCASE)
       ADD_SECTION("\\*+ *Tasks? *\\*\\*\\*", SECTION_TASK)
       ADD_SECTION("\\*+ *Keywords? *\\*\\*\\*", SECTION_KEYWORD)
       ADD_SECTION("\\*+ *Comments? *\\*\\*\\*", SECTION_COMMENT)})
// clang-format on
{
  m_cmd_delimiter.Set("| || |& & && ; ;; ( ) { }");
}

wex::lex_rfw::~lex_rfw()
{
  delete m_quote;
  delete m_quote_stack;
}

void wex::lex_rfw::keywords_update()
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

void wex::lex_rfw::parse_keyword(
  StyleContext& sc,
  int           cmdState,
  int&          cmdStateNew)
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
    if (cmdState == RFW_CMD_WORD && cmdStateNew != RFW_CMD_SKW_PARTIAL)
    {
      if (strcmp(s, "IN") == 0 && keywordEnds)
        cmdStateNew = RFW_CMD_BODY;
      else
        sc.ChangeState(SCE_SH_IDENTIFIER);

      sc.SetState(SCE_SH_DEFAULT);
      return;
    }

    // detect rfw construct keywords
    if (rfwStruct.InList(s))
    {
      if (cmdState == RFW_CMD_START && keywordEnds)
        cmdStateNew = RFW_CMD_START;
      else if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // ':FOR' needs 'IN' to be highlighted later
    else if (rfwStruct_in.InList(s))
    {
      if (cmdState == RFW_CMD_START && keywordEnds)
        cmdStateNew = RFW_CMD_WORD;
      else if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // disambiguate option items and file test operators
    else if (s[0] == '-')
    {
      if (cmdState != RFW_CMD_TEST && cmdStateNew != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }
    // disambiguate keywords and identifiers
    else if (
      cmdState != RFW_CMD_START || !(m_keywords1.InList(s) && keywordEnds))
    {
      if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }

    if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
      sc.SetState(SCE_SH_DEFAULT);
  }
}

bool wex::lex_rfw::section_keywords_detect(
  const std::string& word,
  StyleContext&      sc,
  int&               cmdStateNew)
{
  std::for_each(
    m_section_keywords.begin(),
    m_section_keywords.end(),
    [&sc](auto& i)
    {
      i.first.match(sc.ch);
    });

  return std::any_of(
    m_section_keywords.begin(),
    m_section_keywords.end(),
    [&word, &sc, &cmdStateNew, this](const auto& i)
    {
      switch (i.first.match_type())
      {
        case wex::regex_part::MATCH_ERROR:
          std::cerr << "error: " << i.first.part() << "\n";
          break;

        case wex::regex_part::MATCH_ALL:
          cmdStateNew = RFW_CMD_SKW_PARTIAL;
          break;

        case wex::regex_part::MATCH_COMPLETE:
          sc.Forward();
          sc.SetState(SCE_SH_WORD);
          cmdStateNew = RFW_CMD_START;

          switch (i.second)
          {
            case SECTION_COMMENT:
              m_sp.comments(sc);
              break;

            case SECTION_KEYWORD:
              if (m_sp.test_case() != -1 && sc.currentPos > m_sp.test_case())
              {
                m_sp.test_case_end(
                  sc.currentPos -
                  static_cast<Sci_PositionU>(sc.LengthCurrent()) - word.size());
              }
              break;

            case SECTION_TESTCASE:
              m_sp.test_case(sc);
              break;

            default:; // do nothing
          }
          return true;

        default:; // do nothing
      }

      return false;
    });
}

bool wex::lex_rfw::spaced_keywords_detect(
  const std::string& word,
  StyleContext&      sc,
  int&               cmdStateNew)
{
  return std::any_of(
    m_spaced_keywords.begin(),
    m_spaced_keywords.end(),
    [&sc, &word, &cmdStateNew](const auto& i)
    {
      if (std::equal(word.begin(), word.end(), i.begin()))
      {
        if (word.size() == i.size())
        {
          sc.Forward();
          sc.SetState(SCE_SH_WORD);
          cmdStateNew = RFW_CMD_START;
          return true;
        }
        else
        {
          cmdStateNew = RFW_CMD_SKW_PARTIAL;
        }
      }
      return false;
    });
}

void wex::lex_rfw::state_check(
  StyleContext& sc,
  int           cmdState,
  int&          cmdStateNew,
  LexAccessor&  styler)
{
  const CharacterSet setParam(CharacterSet::setAlphaNum, "$@_");

  int digit;

  // Determine if the current state should terminate.
  switch (sc.state)
  {
    case SCE_SH_OPERATOR:
      sc.SetState(SCE_SH_DEFAULT);
      if (cmdState == RFW_CMD_DELIM) // if command delimiter, start new
                                     // command
        cmdStateNew = RFW_CMD_START;
      else if (sc.chPrev == '\\') // propagate command state if line continued
        cmdStateNew = cmdState;
      break;

    case SCE_SH_TESTCASE:
      if (!setWord.Contains(sc.ch))
      {
        cmdStateNew = RFW_CMD_START;
        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_WORD:
    case SCE_SH_WORD2:
      parse_keyword(sc, cmdState, cmdStateNew);
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
      else if (cmdState == RFW_CMD_ARITH && !setWordStart.Contains(sc.ch))
      {
        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_NUMBER:
      digit = wex::lex_rfw_access(styler).translate_digit(sc.ch);

      if (numBase == RFW_BASE_DECIMAL)
      {
        if (sc.ch == '#')
        {
          char s[10];
          sc.GetCurrent(s, sizeof(s));
          numBase = wex::lex_rfw_access(styler).number_base(s);
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
      if (
        m_visual_mode && sc.atLineEnd && sc.chPrev != '\\' &&
        sc.currentPos < m_sp.comments())
      {
        sc.SetState(SCE_SH_DEFAULT);
      }
      break;

    case SCE_SH_SCALAR: // variable names
      if (!setParam.Contains(sc.ch))
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

bool wex::lex_rfw::state_check_continue(
  StyleContext& sc,
  int&          cmdState,
  LexAccessor&  styler)
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
    sc.SetState(cmdState == RFW_CMD_TESTCASE ? SCE_SH_TESTCASE : SCE_SH_WORD);
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
    if (cmdState == RFW_CMD_ARITH)
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
    if (cmdState != RFW_CMD_ARITH && sc.ch == '(' && sc.chNext != '(')
    {
      int i = wex::lex_rfw_access(styler).glob_scan(sc);
      if (i > 1)
      {
        sc.SetState(SCE_SH_IDENTIFIER);
        sc.Forward(i);
        return true;
      }
    }
    // handle opening delimiters for test/arithmetic expressions - ((,[[,[
    if (cmdState == RFW_CMD_START || cmdState == RFW_CMD_BODY)
    {
      if (sc.Match('(', '('))
      {
        cmdState = RFW_CMD_ARITH;
        sc.Forward();
      }
      else if (sc.Match('[', '[') && IsASpace(sc.GetRelative(2)))
      {
        cmdState     = RFW_CMD_TEST;
        testExprType = 1;
        sc.Forward();
      }
      else if (sc.ch == '[' && IsASpace(sc.chNext))
      {
        cmdState     = RFW_CMD_TEST;
        testExprType = 2;
      }
    }
    // section state -- for ((x;y;z)) in ... looping
    if (cmdState == RFW_CMD_WORD && sc.Match('(', '('))
    {
      cmdState = RFW_CMD_ARITH;
      sc.Forward();
      return true;
    }
    // handle command delimiters in command START|BODY|WORD state, also TEST
    // if 'test'
    if (
      cmdState == RFW_CMD_START || cmdState == RFW_CMD_BODY ||
      cmdState == RFW_CMD_WORD ||
      (cmdState == RFW_CMD_TEST && testExprType == 0))
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
        cmdState = RFW_CMD_DELIM;
        return true;
      }
    }
    // handle closing delimiters for test/arithmetic expressions - )),]],]
    if (cmdState == RFW_CMD_ARITH && sc.Match(')', ')'))
    {
      cmdState = RFW_CMD_BODY;
      sc.Forward();
    }
    else if (cmdState == RFW_CMD_TEST && IsASpace(sc.chPrev))
    {
      if (sc.Match(']', ']') && testExprType == 1)
      {
        sc.Forward();
        cmdState = RFW_CMD_BODY;
      }
      else if (sc.ch == ']' && testExprType == 2)
      {
        cmdState = RFW_CMD_BODY;
      }
    }
  }

  return false;
}

void SCI_METHOD wex::lex_rfw::Fold(
  Sci_PositionU startPos,
  Sci_Position  length,
  int,
  IDocument* pAccess)
{
  LexAccessor styler(pAccess);

  Sci_PositionU endPos      = startPos + length;
  Sci_Position  lineCurrent = styler.GetLine(startPos);

  int levelPrev    = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK,
      levelCurrent = levelPrev, visibleChars = 0;

  char chNext = styler[startPos];

  for (Sci_PositionU i = startPos; i < endPos; i++)
  {
    char ch = chNext;
    chNext  = styler.SafeGetCharAt(i + 1);

    const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

    wex::lex_rfw_access rfw(styler, lineCurrent);

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
      if (lev != styler.LevelAt(lineCurrent))
      {
        styler.SetLevel(lineCurrent, lev);
      }
      lineCurrent++;
      levelPrev    = levelCurrent;
      visibleChars = 0;
    }

    if (!isspacechar(ch))
      visibleChars++;
  }

  // Fill in the real level of the next line, keeping the current flags as they
  // will be filled in later
  int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
  styler.SetLevel(lineCurrent, levelPrev | flagsNext);
}

void SCI_METHOD wex::lex_rfw::Lex(
  Sci_PositionU startPos,
  Sci_Position  length,
  int           initStyle,
  IDocument*    pAccess)
{
  LexAccessor styler(pAccess);

  m_quote       = new quote(styler);
  m_quote_stack = new quote_stack(styler);

  bool pipes    = false;
  int  cmdState = RFW_CMD_START;

  Sci_PositionU endPos = startPos + length;
  Sci_Position  ln     = wex::lex_rfw_access(styler).init(startPos);
  initStyle            = SCE_SH_DEFAULT;

  std::string words;

  StyleContext sc(startPos, endPos - startPos, initStyle, styler);

  for (; sc.More(); sc.Forward())
  {
    if (sc.ch == '|' && sc.atLineStart)
    {
      pipes = true;
    }

    // handle line continuation, updates per-line stored state
    if (sc.atLineStart)
    {
      ln = styler.GetLine(sc.currentPos);
      if (
        sc.state == SCE_SH_STRING || sc.state == SCE_SH_BACKTICKS ||
        sc.state == SCE_SH_CHARACTER || sc.state == SCE_SH_COMMENTLINE ||
        sc.state == SCE_SH_PARAM)
      {
        // force backtrack while retaining cmdState
        styler.SetLineState(ln, RFW_CMD_BODY);
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
            cmdState = RFW_CMD_START;
        }
        styler.SetLineState(ln, cmdState);
      }
    }

    // controls change of cmdState at the end of a non-whitespace element
    // states BODY|TEST|ARITH persist until the end of a command segment
    // state WORD persist, but ends with 'in' or 'do' construct keywords
    int cmdStateNew = RFW_CMD_BODY;
    if (
      cmdState == RFW_CMD_TEST || cmdState == RFW_CMD_ARITH ||
      cmdState == RFW_CMD_WORD)
      cmdStateNew = cmdState;

    m_style_prev = sc.state;

    words.append(1, sc.ch);

    if (!spaced_keywords_detect(words, sc, cmdStateNew))
    {
      section_keywords_detect(words, sc, cmdStateNew);

      if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
      {
        words.clear();

        std::for_each(
          m_section_keywords.begin(),
          m_section_keywords.end(),
          [&sc](auto& i)
          {
            i.first.reset();
            i.first.match(sc.ch);
          });
      }
    }

    if (
      (sc.chPrev == '|' && sc.ch == ' ') && cmdState != RFW_CMD_TESTCASE &&
      cmdState != RFW_CMD_WORD)
    {
      cmdState = RFW_CMD_START;
    }

    state_check(sc, cmdState, cmdStateNew, styler);

    // update cmdState about the current command segment
    if (m_style_prev != SCE_SH_DEFAULT && sc.state == SCE_SH_DEFAULT)
    {
      cmdState = cmdStateNew;
    }
    else if (pipes && sc.ch == '|' && sc.currentPos > m_sp.test_case())
    {
      if (m_sp.test_case_end() != -1 && sc.currentPos > m_sp.test_case_end())
      {
        cmdState = RFW_CMD_START;
      }
      else
      {
        cmdState = (sc.atLineStart ? RFW_CMD_TESTCASE : RFW_CMD_START);
      }
    }
    else if (
      m_visual_mode && !pipes && sc.ch != '#' && !isspace(sc.ch) &&
      sc.atLineStart && sc.currentPos > m_sp.test_case())
    {
      if (m_sp.test_case_end() == -1 || sc.currentPos < m_sp.test_case_end())
      {
        cmdState = RFW_CMD_TESTCASE;
      }

      if (sc.currentPos > m_sp.comments())
      {
        sc.SetState(SCE_SH_COMMENTLINE);
      }
    }

    if (sc.state == SCE_SH_DEFAULT)
    {
      if (state_check_continue(sc, cmdState, styler))
      {
        continue;
      }
    }
  }

  sc.Complete();
}

void SCI_METHOD wex::lex_rfw::Release()
{
  delete this;
}

Sci_Position SCI_METHOD wex::lex_rfw::WordListSet(int n, const char* wl)
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
  wex::lex_rfw::language(),
  wex::lex_rfw::get,
  wex::lex_rfw::name(),
  wex::option_set_rfw::keywords());
