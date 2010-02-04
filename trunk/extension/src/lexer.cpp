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
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExAlignText

wxExLexer::wxExLexer(const wxXmlNode* node)
{
  m_CommentBegin.clear();
  m_CommentBegin2.clear();
  m_CommentEnd.clear();
  m_CommentEnd2.clear();
  m_Colourings.clear();
  m_Extensions.clear();
  m_Properties.clear();
  m_Keywords.clear();
  m_KeywordsSet.clear();
  m_ScintillaLexer.clear();

  if (node != NULL)
  {
    Set(node);
  }
}

const wxString wxExLexer::GetFormattedText(
  const wxString& lines,
  const wxString& header,
  bool fill_out_with_space,
  bool fill_out) const
{
  wxString text = lines, header_to_use = header;
  size_t nCharIndex;

  wxString out;

  // Process text between the carriage return line feeds.
  while ((nCharIndex = text.find("\n")) != wxString::npos)
  {
    out << wxExAlignText(
      text.substr(0, nCharIndex),
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);

    text = text.substr(nCharIndex + 1);
    header_to_use = wxString(' ', header.size());
  }

  if (!text.empty())
  {
    out << wxExAlignText(
      text,
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);
  }

  return out;
}

const wxString wxExLexer::GetKeywordsString(int keyword_set) const
{
  if (keyword_set == -1)
  {
    return GetKeywordsStringSet(m_Keywords);
  }
  else
  {
    std::map< int, std::set<wxString> >::const_iterator it = 
      m_KeywordsSet.find(keyword_set);

    if (it != m_KeywordsSet.end())
    {
      return GetKeywordsStringSet(it->second);
    }
  }

  return wxEmptyString;
}

const wxString wxExLexer::GetKeywordsStringSet(
  const std::set<wxString>& kset) const
{
  wxString keywords;

  for (
    std::set<wxString>::const_iterator it = kset.begin();
    it != kset.end();
    ++it)
  {
    keywords += *it + " ";
  }

  return keywords.Trim(); // remove the ending space
}

bool wxExLexer::IsKeyword(const wxString& word) const
{
  std::set<wxString>::const_iterator it = m_Keywords.find(word);
  return (it != m_Keywords.end());
}

bool wxExLexer::KeywordStartsWith(const wxString& word) const
{
  std::set<wxString>::const_iterator it = m_Keywords.lower_bound(word.Lower());
  return 
    it != m_Keywords.end() &&
    it->StartsWith(word.Lower());
}

const wxString wxExLexer::MakeComment(
  const wxString& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  wxString out;

  text.find("\n") != wxString::npos ?
    out << GetFormattedText(text, wxEmptyString, fill_out_with_space, fill_out):
    out << wxExAlignText(text, wxEmptyString, fill_out_with_space, fill_out, *this);

  return out;
}

const wxString wxExLexer::MakeComment(
  const wxString& prefix,
  const wxString& text) const
{
  wxString out;

  text.find("\n") != wxString::npos ?
    out << GetFormattedText(text, prefix, true, true):
    out << wxExAlignText(text, prefix, true, true, *this);

  return out;
}

const wxString wxExLexer::MakeSingleLineComment(
  const wxString& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  if (m_CommentBegin.empty() && m_CommentEnd.empty())
  {
    return text;
  }

  // First set the fill_out_character.
  wxUniChar fill_out_character;

  if (fill_out_with_space || m_ScintillaLexer == "hypertext")
  {
    fill_out_character = ' ';
  }
  else
  {
    if (text.empty())
    {
      if (m_CommentBegin == m_CommentEnd)
           fill_out_character = '-';
      else fill_out_character = m_CommentBegin[m_CommentBegin.size() - 1];
    }
    else   fill_out_character = ' ';
  }

  wxString out = m_CommentBegin + fill_out_character + text;

  // Fill out characters.
  if (fill_out)
  {
    // To prevent filling out spaces
    if (fill_out_character != ' ' || !m_CommentEnd.empty())
    {
      const int fill_chars = UsableCharactersPerLine() - text.size();

      if (fill_chars > 0)
      {
        const wxString fill_out(fill_out_character, fill_chars);
        out += fill_out;
      }
    }
  }

  if (!m_CommentEnd.empty()) out += fill_out_character + m_CommentEnd;

  return out;
}

void wxExLexer::Set(const wxXmlNode* node)
{
  m_ScintillaLexer = node->GetAttribute("name", "");
  m_Extensions = node->GetAttribute(
    "extensions", 
    "*." + m_ScintillaLexer);

  if (node->GetAttribute("match", "") != "")
  {
    m_Colourings = 
      wxExLexers::Get()->AutoMatch(node->GetAttribute("match", ""));
  }

  if (m_ScintillaLexer == "hypertext")
  {
    // As our lexers.xml files cannot use xml comments,
    // add them here.
    m_CommentBegin = "<!--";
    m_CommentEnd = "-->";
  }

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "colourings")
    {
      const std::vector<wxString> v = 
        wxExLexers::Get()->ParseTagColourings(child);

      m_Colourings.insert(
        m_Colourings.end(), 
        v.begin(), v.end());
    }
    else if (child->GetName() == "keywords")
    {
      if (!SetKeywords(child->GetNodeContent().Strip(wxString::both)))
      {
        wxLogError(
          _("Keywords could not be set on line: %d"), 
          child->GetLineNumber());
      }
    }
    else if (child->GetName() == "properties")
    {
      m_Properties = wxExLexers::Get()->ParseTagProperties(child);
    }
    else if (child->GetName() == "comments")
    {
      m_CommentBegin = child->GetAttribute("begin1", "");
      m_CommentEnd = child->GetAttribute("end1", "");
      m_CommentBegin2 = child->GetAttribute("begin2", "");
      m_CommentEnd2 = child->GetAttribute("end2", "");
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined lexer tag: %s on line: %d"),
        child->GetName().c_str(), 
        child->GetLineNumber());
    }

    child = child->GetNext();
  }
}

bool wxExLexer::SetKeywords(const wxString& value)
{
  if (!m_Keywords.empty())
  {
    m_Keywords.clear();
  }

  if (!m_KeywordsSet.empty())
  {
    m_KeywordsSet.clear();
  }

  std::set<wxString> keywords_set;

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

int wxExLexer::UsableCharactersPerLine() const
{
  // We always use lines with 80 characters. We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return 80
    - ((m_CommentBegin.size() != 0) ? m_CommentBegin.size() + 1 : 0)
    - ((m_CommentEnd.size() != 0) ? m_CommentEnd.size() + 1 : 0);
}
