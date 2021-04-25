////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw.cpp
// Purpose:   Implementation of lmRFW
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <string>
#include <vector>

#include "lex-rfw.h"

void parse_keyword(
  const CharacterSet& setWord,
  const WordList&     cmdDelimiter,
  WordList*           keywordlists[],
  StyleContext&       sc,
  int                 cmdState,
  int&                cmdStateNew)
{
  const WordList& keywords1 = *keywordlists[0];

  WordList rfwStruct;
  rfwStruct.Set("");

  WordList rfwStruct_in;
  rfwStruct_in.Set(":FOR FOR");

  // "." never used in RFW variable names but used in file names
  if (!setWord.Contains(sc.ch))
  {
    char s[500];
    char s2[10];
    sc.GetCurrent(s, sizeof(s));

    // allow keywords ending in a whitespace or command delimiter
    s2[0] = static_cast<char>(sc.ch);
    s2[1] = '\0';

    const bool keywordEnds = IsASpace(sc.ch) || cmdDelimiter.InList(s2);

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
    else if (cmdState != RFW_CMD_START || !(keywords1.InList(s) && keywordEnds))
    {
      if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
        sc.ChangeState(SCE_SH_IDENTIFIER);
    }

    if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
      sc.SetState(SCE_SH_DEFAULT);
  }
};

static void colourise(
  Sci_PositionU startPos,
  Sci_Position  length,
  int           initStyle,
  WordList*     keywordlists[],
  Accessor&     styler)
{
  WordList cmdDelimiter;
  cmdDelimiter.Set("| || |& & && ; ;; ( ) { }");

  std::vector<std::string> sections;
  sections.emplace_back("Settings");
  sections.emplace_back("Variables");
  sections.emplace_back("Test Cases");
  sections.emplace_back("Tasks");
  sections.emplace_back("Keywords");
  sections.emplace_back("Comments");
  
  std::vector<std::string> special_keywords;
  
  for (const auto& section : sections)
  {
    special_keywords.emplace_back("*" + section + "*");
    special_keywords.emplace_back("* " + section + " *");
    special_keywords.emplace_back("** " + section + " **");
    special_keywords.emplace_back("*** " + section + " ***");
  }
  
  const WordList& keywords2 = *keywordlists[1];
  
  bool visual_mode = true;

  for (int i = 0; i < keywords2.Length(); i++)
  {
    std::string keyword(keywords2.WordAt(i));

    for (int j = 0; j < keyword.size(); j++)
    {
      if (keyword[j] == '_')
      {
        keyword[j] = ' ';
      }
    }
    
    if (keyword != "EX")
    {
      special_keywords.push_back(keyword);
    }
    else
    {
      visual_mode = false;
    }
  }

  const CharacterSet setWordStart(CharacterSet::setAlpha, ":_[*");
  const CharacterSet setWordStartTSV(CharacterSet::setAlphaNum, ":_[*");
  // note that [+-] are often parts of identifiers in shell scripts
  const CharacterSet setWord(CharacterSet::setAlphaNum, "._+-]*");
  const CharacterSet setMetaCharacter(CharacterSet::setNone, "|&;()<> \t\r\n");
  const CharacterSet setRFWOperator(
    CharacterSet::setNone,
    "^&%()-+={};>,/<?!.~@");
  const CharacterSet setSingleCharOp(
    CharacterSet::setNone,
    "rwxoRWXOezsfdlpSbctugkTBMACahGLNn");
  const CharacterSet setParam(CharacterSet::setAlphaNum, "$@_");
  const CharacterSet setLeftShift(CharacterSet::setDigits, "$");

  wex::quote       quote(styler);
  wex::quote_stack quoteStack(styler);

  bool pipes   = false;
  int  numBase = 0, digit, cmdState = RFW_CMD_START, testExprType = 0;

  static Sci_PositionU commentsSectionPos    = -1;
  static Sci_PositionU testCaseSectionPos    = -1;
  static Sci_PositionU testCaseSectionEndPos = -1;

  Sci_PositionU endPos = startPos + length;
  Sci_Position  ln     = wex::lex_rfw(styler).init(startPos);
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
    int stylePrev = sc.state;

    // detect special keywords
    words.append(1, sc.ch);
    for (unsigned ki = 0; ki < special_keywords.size(); ki++)
    {
      if (std::equal(words.begin(), words.end(), special_keywords[ki].begin()))
      {
        if (words.size() == special_keywords[ki].size())
        {
          sc.Forward();
          sc.SetState(SCE_SH_WORD);
          cmdStateNew = RFW_CMD_START;

          /* The recommended header format is *** Settings ***,
             but the header is case-insensitive, surrounding spaces are
             optional, and the number of asterisk characters can vary as long
             as there is one asterisk in the beginning.
             In addition to using the plural format, also singular variants
             like Setting and Test Case are accepted.
             In other words, also *setting would be recognized as a section
             header.
            */
          std::match_results<std::string::const_iterator> m;
          std::string re_tc("\\*+ *Test Cases? *\\**");
          std::string re_keyw("\\*+ *Keywords? *\\**");
          std::string re_comm("\\*+ *Comments? *\\**");

          if (std::regex_search(words, m, std::regex(re_tc, std::regex::icase)))
          {
            testCaseSectionPos = sc.currentPos;
          }
          else if (
            testCaseSectionPos != -1 && sc.currentPos > testCaseSectionPos &&
            std::regex_search(words, m, std::regex(re_keyw, std::regex::icase)))
          {
            testCaseSectionEndPos =
              sc.currentPos - sc.LengthCurrent() - std::string(m[0]).size();
          }
          else if (
            std::regex_search(words, m, std::regex(re_comm, std::regex::icase)))
          {
            commentsSectionPos = sc.currentPos;
          }

          break;
        }
        else
        {
          cmdStateNew = RFW_CMD_SKW_PARTIAL;
        }
      }
    }

    if (cmdStateNew != RFW_CMD_SKW_PARTIAL)
      words.clear();

    if (
      (sc.chPrev == '|' && sc.ch == ' ') && cmdState != RFW_CMD_TESTCASE &&
      cmdState != RFW_CMD_WORD)
    {
      cmdState = RFW_CMD_START;
    }

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
        parse_keyword(
          setWord,
          cmdDelimiter,
          keywordlists,
          sc,
          cmdState,
          cmdStateNew);
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
        digit = wex::lex_rfw(styler).translate_digit(sc.ch);

        if (numBase == RFW_BASE_DECIMAL)
        {
          if (sc.ch == '#')
          {
            char s[10];
            sc.GetCurrent(s, sizeof(s));
            numBase = wex::lex_rfw(styler).number_base(s);
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
        if (visual_mode && sc.atLineEnd && sc.chPrev != '\\' && sc.currentPos < commentsSectionPos)
        {
          sc.SetState(SCE_SH_DEFAULT);
        }
        break;

      case SCE_SH_SCALAR: // variable names
        if (!setParam.Contains(sc.ch))
        {
          if (sc.LengthCurrent() == 1)
          {
            // Special variable: $(, $_ etc.
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
        if (sc.ch == '\\' && quoteStack.up() != '\\')
        {
          if (quoteStack.style() != RFW_DELIM_LITERAL)
            sc.Forward();
        }
        else if (sc.ch == quoteStack.down())
        {
          quoteStack.decrease();

          if (quoteStack.count() == 0)
          {
            if (quoteStack.depth() > 0)
            {
              quoteStack.pop();
            }
            else
              sc.ForwardSetState(SCE_SH_DEFAULT);
          }
        }
        else if (sc.ch == quoteStack.up())
        {
          quoteStack.increase();
        }
        else
        {
          if (
            quoteStack.style() == RFW_DELIM_STRING ||
            quoteStack.style() == RFW_DELIM_LSTRING)
          { // do nesting for "string", $"locale-string"
            if (sc.ch == '`')
            {
              quoteStack.push(sc.ch, RFW_DELIM_BACKTICK);
            }
            else if (sc.ch == '$' && sc.chNext == '(')
            {
              sc.Forward();
              quoteStack.push(sc.ch, RFW_DELIM_COMMAND);
            }
          }
          else if (
            quoteStack.style() == RFW_DELIM_COMMAND ||
            quoteStack.style() == RFW_DELIM_BACKTICK)
          { // do nesting for $(command), `command`
            if (sc.ch == '\'')
            {
              quoteStack.push(sc.ch, RFW_DELIM_LITERAL);
            }
            else if (sc.ch == '\"')
            {
              quoteStack.push(sc.ch, RFW_DELIM_STRING);
            }
            else if (sc.ch == '`')
            {
              quoteStack.push(sc.ch, RFW_DELIM_BACKTICK);
            }
            else if (sc.ch == '$')
            {
              if (sc.chNext == '\'')
              {
                sc.Forward();
                quoteStack.push(sc.ch, RFW_DELIM_CSTRING);
              }
              else if (sc.chNext == '\"')
              {
                sc.Forward();
                quoteStack.push(sc.ch, RFW_DELIM_LSTRING);
              }
              else if (sc.chNext == '(')
              {
                sc.Forward();
                quoteStack.push(sc.ch, RFW_DELIM_COMMAND);
              }
            }
          }
        }
        break;

      case SCE_SH_PARAM: // ${parameter}
        if (sc.ch == '\\' && quote.up() != '\\')
        {
          sc.Forward();
        }
        else if (sc.ch == quote.down())
        {
          quote.decrease();

          if (quote.count() == 0)
          {
            sc.ForwardSetState(SCE_SH_DEFAULT);
          }
        }
        else if (sc.ch == quote.up())
        {
          quote.increase();
        }
        break;

      case SCE_SH_CHARACTER: // singly-quoted strings
        if (sc.ch == quote.down())
        {
          quote.decrease();
          if (quote.count() == 0)
          {
            sc.ForwardSetState(SCE_SH_DEFAULT);
          }
        }
        break;
    }

    // update cmdState about the current command segment
    if (stylePrev != SCE_SH_DEFAULT && sc.state == SCE_SH_DEFAULT)
    {
      cmdState = cmdStateNew;
    }
    else if (pipes && sc.ch == '|' && sc.currentPos > testCaseSectionPos)
    {
      if (testCaseSectionEndPos != -1 && sc.currentPos > testCaseSectionEndPos)
      {
        cmdState = RFW_CMD_START;
      }
      else
      {
        cmdState = (sc.atLineStart ? RFW_CMD_TESTCASE : RFW_CMD_START);
      }
    }
    else if (
      visual_mode &&
      !pipes && sc.ch != '#' && !isspace(sc.ch) && sc.atLineStart &&
      sc.currentPos > testCaseSectionPos)
    {
      if (testCaseSectionEndPos == -1 || sc.currentPos < testCaseSectionEndPos)
      {
        cmdState = RFW_CMD_TESTCASE;
      }
      
      if (sc.currentPos > commentsSectionPos)
      {
        sc.SetState(SCE_SH_COMMENTLINE);
      }
    }

    // Determine if a new state should be entered.
    if (sc.state == SCE_SH_DEFAULT)
    {
      if (sc.ch == '\\')
      {
        // RFW can escape any non-newline as a literal
        sc.SetState(SCE_SH_IDENTIFIER);
        if (sc.chNext == '\r' || sc.chNext == '\n')
          sc.SetState(SCE_SH_OPERATOR);
      }
      else if (setWordStartTSV.Contains(sc.ch))
      {
        // TODO: or set to SCE_SH_WORD2
        sc.SetState(
          cmdState == RFW_CMD_TESTCASE ? SCE_SH_TESTCASE : SCE_SH_WORD);
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
          stylePrev != SCE_SH_WORD && stylePrev != SCE_SH_WORD2 &&
          stylePrev != SCE_SH_IDENTIFIER &&
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
      else if (sc.ch == '\"')
      {
        sc.SetState(SCE_SH_STRING);
        quoteStack.start(sc.ch, RFW_DELIM_STRING);
      }
      else if (sc.ch == '`')
      {
        sc.SetState(SCE_SH_BACKTICKS);
        quoteStack.start(sc.ch, RFW_DELIM_BACKTICK);
      }
      else if (sc.ch == '$' || sc.ch == '@')
      {
        if (sc.Match("$(("))
        {
          sc.SetState(SCE_SH_OPERATOR); // handle '((' later
          continue;
        }
        sc.SetState(SCE_SH_SCALAR);
        sc.Forward();
        if (sc.ch == '\'')
        {
          sc.ChangeState(SCE_SH_STRING);
          quoteStack.start(sc.ch, RFW_DELIM_CSTRING);
        }
        else if (sc.ch == '"')
        {
          sc.ChangeState(SCE_SH_STRING);
          quoteStack.start(sc.ch, RFW_DELIM_LSTRING);
        }
        else if (sc.ch == '(')
        {
          sc.ChangeState(SCE_SH_BACKTICKS);
          quoteStack.start(sc.ch, RFW_DELIM_COMMAND);
        }
        else if (sc.ch == '`')
        { // $` seen in a configure script, valid?
          sc.ChangeState(SCE_SH_BACKTICKS);
          quoteStack.start(sc.ch, RFW_DELIM_BACKTICK);
        }
        else
        {
          continue; // scalar has no delimiter pair
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
          int i = wex::lex_rfw(styler).glob_scan(sc);
          if (i > 1)
          {
            sc.SetState(SCE_SH_IDENTIFIER);
            sc.Forward(i);
            continue;
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
        // special state -- for ((x;y;z)) in ... looping
        if (cmdState == RFW_CMD_WORD && sc.Match('(', '('))
        {
          cmdState = RFW_CMD_ARITH;
          sc.Forward();
          continue;
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
            isCmdDelim = cmdDelimiter.InList(s);
            if (isCmdDelim)
              sc.Forward();
          }
          if (!isCmdDelim)
          {
            s[1]       = '\0';
            isCmdDelim = cmdDelimiter.InList(s);
          }
          if (isCmdDelim)
          {
            cmdState = RFW_CMD_DELIM;
            continue;
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
    } // sc.state
  }
  sc.Complete();
}

static void fold(
  Sci_PositionU startPos,
  Sci_Position  length,
  int,
  WordList*[],
  Accessor& styler)
{
  const bool foldComment = styler.GetPropertyInt("fold.comment") != 0;
  const bool foldPipes   = styler.GetPropertyInt("fold.pipes") != 0;
  const bool foldTabs    = styler.GetPropertyInt("fold.tabs") != 0;
  const bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

  Sci_PositionU endPos      = startPos + length;
  Sci_Position  lineCurrent = styler.GetLine(startPos);

  int levelPrev    = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK,
      levelCurrent = levelPrev, styleNext = styler.StyleAt(startPos),
      visibleChars = 0;

  char chNext = styler[startPos];

  for (Sci_PositionU i = startPos; i < endPos; i++)
  {
    char ch   = chNext;
    chNext    = styler.SafeGetCharAt(i + 1);
    int style = styleNext;
    styleNext = styler.StyleAt(i + 1);

    const bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

    wex::lex_rfw rfw(styler, lineCurrent);

    // Comment folding
    if (foldComment && atEOL && rfw.is_comment_line())
    {
      if (!rfw.is_comment_line(-1) && rfw.is_comment_line(1))
        levelCurrent++;
      else if (rfw.is_comment_line(-1) && !rfw.is_comment_line(1))
        levelCurrent--;
    }

    // Pipe folding
    if (foldPipes && atEOL && rfw.is_pipe_line())
    {
      if (!rfw.is_pipe_line(-1) && rfw.is_pipe_line(1))
        levelCurrent++;
      else if (rfw.is_pipe_line(-1) && !rfw.is_pipe_line(1))
        levelCurrent--;
    }

    // Tab folding
    if (foldTabs && atEOL && rfw.is_tab_line())
    {
      if (!rfw.is_tab_line(-1) && rfw.is_tab_line(1))
        levelCurrent++;
      else if (rfw.is_tab_line(-1) && !rfw.is_tab_line(1))
        levelCurrent--;
    }

    if (atEOL)
    {
      int lev = levelPrev;
      if (visibleChars == 0 && foldCompact)
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

static const char* const keywords[] = {
  "Primary Keywords",
  "Secondary Keywords",
  0};

LexerModule lmRFW(SCLEX_AUTOMATIC, colourise, "rfw", fold, keywords);
