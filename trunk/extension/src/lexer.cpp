/******************************************************************************\
* File:          lexer.cpp
* Purpose:       Implementation of lexer classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stc/stc.h> // for wxSTC_KEYWORDSET_MAX
#include <wx/tokenzr.h>
#include <wx/extension/lexer.h>

using namespace std;

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
    std::map< int, std::set<wxString> >::const_iterator it = m_KeywordsSet.find(keyword_set);

    if (it == m_KeywordsSet.end())
    {
      wxFAIL;
    }
    else
    {
      set<wxString> theset = it->second;

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

const wxString exLexer::MakeComment(
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

bool exLexer::SetKeywords(const wxString& value)
{
  set<wxString> keywords_set;

  wxStringTokenizer tkz(value, "\r\n ");

  int setno = 0;

  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken();
    wxStringTokenizer fields(line, ":");

    wxString keyword;

    if (fields.CountTokens() > 1)
    {
      keyword = fields.GetNextToken();

      const int new_setno = atoi(fields.GetNextToken().c_str());

      if (new_setno >= wxSTC_KEYWORDSET_MAX)
      {
        return false;
      }

      if (new_setno != setno)
      {
        if (!keywords_set.empty())
        {
          m_KeywordsSet.insert(make_pair(setno, keywords_set));
          keywords_set.clear();
        }

        setno = new_setno;
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

  m_KeywordsSet.insert(make_pair(setno, keywords_set));

  return true;
}

int exLexer::UsableCharactersPerLine() const
{
  // We always use lines with 80 characters. We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return 80
      -  (m_CommentBegin.length() + 1)
      - ((m_CommentEnd.length() != 0) ? m_CommentEnd.length() + 1 : 0);
}
