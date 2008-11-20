/******************************************************************************\
* File:          lexer.cpp
* Purpose:       Implementation of lexer classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tokenzr.h>
#include <wx/extension/extension.h>

exLexer::exSyntaxType exLexer::m_SyntaxType = exLexer::SYNTAX_NONE;
exLexer::exSyntaxType exLexer::m_LastSyntaxType = exLexer::SYNTAX_NONE;

using namespace std;

exLexer::exCommentType CheckCommentSyntax(
  const wxString& syntax_begin,
  const wxString& syntax_end,
  wxChar c1,
  wxChar c2)
{
  const wxString comp = ((syntax_begin.length() == 1) ? wxString(c1) : wxString(c2) + wxString(c1));

  if (syntax_begin == comp)
  {
    return (syntax_end == comp) ? exLexer::COMMENT_BOTH: exLexer::COMMENT_BEGIN;
  }
  else
  {
    if (syntax_end == comp ||
        // If syntax_end was empty, we assume the terminating 0 ends the comment.
       (syntax_end.empty() && c1 == 0))
    {
      return exLexer::COMMENT_END;
    }
  }

  if ((syntax_begin.length() > 1 && syntax_begin[0] == c1) ||
      (syntax_end.length() > 1 && syntax_end[0] == c1) ||
      (c1 == 0))
  {
    return exLexer::COMMENT_INCOMPLETE;
  }

  return exLexer::COMMENT_NONE;
}

exLexer::exCommentType exLexer::CheckForComment(wxChar c1, wxChar c2) const
{
  if (m_CommentBegin2.empty())
  {
    return CheckCommentSyntax(m_CommentBegin, m_CommentEnd, c1, c2);
  }

  exCommentType comment_type1 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE)
  {
    if ((comment_type1 = CheckCommentSyntax(m_CommentBegin, m_CommentEnd, c1, c2)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_ONE;
  }

  exCommentType comment_type2 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_TWO)
  {
    if ((comment_type2 = CheckCommentSyntax(m_CommentBegin2, m_CommentEnd2, c1, c2)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_TWO;
  }

  exCommentType comment_type = COMMENT_NONE;

  switch (comment_type1)
  {
  case COMMENT_NONE:  comment_type = comment_type2; break;
  case COMMENT_BEGIN: comment_type = COMMENT_BEGIN; break;
  case COMMENT_END:   comment_type = COMMENT_END; break;
  case COMMENT_BOTH:  comment_type = COMMENT_BOTH; break;
  case COMMENT_INCOMPLETE:
    comment_type = (comment_type2 == COMMENT_NONE) ? COMMENT_INCOMPLETE: comment_type2;
    break;
  default: wxLogError(FILE_INFO("Unhandled"));
  }

  if (comment_type == COMMENT_END)
  {
    // E.g. we have a correct /* */ comment, with */ at the end of the line.
    // Then the end of line itself should not generate a COMMENT_END.
    if (m_SyntaxType == SYNTAX_NONE) comment_type = COMMENT_NONE;
    // Keep the syntax type.
    m_LastSyntaxType = m_SyntaxType;
    m_SyntaxType = SYNTAX_NONE;
  }

  return comment_type;
}

const exLexer exLexer::Default() const
{
  exLexer lexer;
  lexer.m_ScintillaLexer = "cpp";
  lexer.m_CommentBegin = "/*";
  lexer.m_CommentEnd = "*/";
  lexer.m_CommentBegin2 = "//";
  return lexer;
}

const wxString exLexer::FormatText(
  const wxString& text,
  bool fill_out,
  bool fill_out_with_space) const
{
  // First set the fill_out_character.
  wxChar fill_out_character;

  if (fill_out_with_space)  fill_out_character = ' ';
  else
  {
    if (text.empty())
    {
      if (m_CommentBegin == m_CommentEnd || m_CommentEnd.empty())
           fill_out_character = '-';
      else fill_out_character = m_CommentBegin[m_CommentBegin.length() - 1];
    }
    else   fill_out_character = ' ';
  }

  wxString out = m_CommentBegin + fill_out_character + text;

  // Add fill out characters if necessary.
  if (fill_out)
  {
    const wxString fill_out(fill_out_character, UsableCharactersPerLine() - text.length());
    out += fill_out;
  }

  if (!m_CommentEnd.empty()) out += fill_out_character + m_CommentEnd;

  return out;
}

const wxString exLexer::GetKeywordsString(int keyword_set) const
{
  wxString keywords;

  if (keyword_set == -1)
  {
    for (
      set<wxString>::const_iterator it = m_Keywords.begin();
      it != m_Keywords.end();
      ++it)
    {
      keywords += *it + " ";
    }
  }
  else
  {
    if (keyword_set >= (int)m_KeywordsSet.size())
    {
      wxLogError(FILE_INFO("Illegal index"));
    }
    else
    {
      set<wxString> theset = m_KeywordsSet.at(keyword_set);
      for (
        set<wxString>::const_iterator it = theset.begin();
        it != theset.end();
        ++it)
      {
        keywords += *it + " ";
      }
    }
  }

  return keywords;
}

bool exLexer::IsKeyword(const wxString& word) const
{
  set<wxString>::const_iterator it = m_Keywords.find(word);
  return (it != m_Keywords.end());
}

bool exLexer::KeywordStartsWith(const wxString& word) const
{
  for (
    set<wxString>::const_iterator it = m_Keywords.begin();
    it != m_Keywords.end();
    ++it)
  {
    if (it->Upper().StartsWith(word.Upper()))
    {
      return true;
    }
  }

  return false;
}

void exLexer::SetKeywords(const wxString& value)
{
  set<wxString> keywords_set;

  wxStringTokenizer tkz(value, "\r\n ");

  int setno = -1;

  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken();
    wxStringTokenizer fields(line, ":");

    wxString keyword;

    if (fields.CountTokens() > 1)
    {
      keyword = fields.GetNextToken();

      const int new_setno = atoi(fields.GetNextToken().c_str());

      if (new_setno != setno)
      {
        setno = new_setno;

        if (!keywords_set.empty())
        {
          m_KeywordsSet.push_back(keywords_set);
          keywords_set.clear();
        }
      }

      keywords_set.insert(keyword);
    }
    else
    {
      keyword = line;
      keywords_set.insert(line);
    }

    m_Keywords.insert(keyword);
  }

  m_KeywordsSet.push_back(keywords_set);
}

void exLexer::SetLexerFromText(const wxString& text)
{
  if (m_ScintillaLexer.empty())
  {
    // Add automatic lexers if text starts with some special tokens.
    const wxString text_lowercase = text.Lower();

    if (text_lowercase.StartsWith("#") ||
        // .po files that do not have comment headers, start with msgid, so set them
        text_lowercase.StartsWith("msgid"))
    {
      (*this) = exApp::GetLexers()->FindByName("bash");
    }
    else if (text_lowercase.StartsWith("<html>") ||
             text_lowercase.StartsWith("<?php") ||
             text_lowercase.StartsWith("<?xml"))
    {
      (*this) = exApp::GetLexers()->FindByName("hypertext");
    }
    // cpp files like #include <map> really do not have a .h extension (e.g. /usr/include/c++/3.3.5/map)
    // so add here.
    else if (text_lowercase.StartsWith("//"))
    {
      (*this) = exApp::GetLexers()->FindByName("cpp");
    }
  }
}

int exLexer::UsableCharactersPerLine() const
{
  // We always use lines with 80 characters. We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return 80
      -  (m_CommentBegin.length() + 1)
      - ((m_CommentEnd.length() != 0) ? m_CommentEnd.length() + 1 : 0);
}
