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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
//#include <numeric> // both for accumulate
//#include <functional>
#include <algorithm>
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

const std::vector<wxExStyle> wxExLexer::AutoMatch(
  const wxString& lexer) const
{
  std::vector<wxExStyle> text;

  auto itlow = 
    wxExLexers::Get()->GetMacros().lower_bound(lexer);

  auto itup = 
    wxExLexers::Get()->GetMacros().upper_bound(lexer + "ZZZ");

  for (
    auto it = itlow;
    it != itup;
    ++it)
  {
    for (
      auto style = 
        wxExLexers::Get()->GetMacrosStyle().begin();
      style != wxExLexers::Get()->GetMacrosStyle().end();
      ++style)
    {
      if (it->first.Contains(style->first))
      {
        text.push_back(wxExStyle(it->second, style->second));
      }
    }
  }

  return text;
}

void wxExLexer::Colourise(wxStyledTextCtrl* stc) const
{
  for_each (m_Colourings.begin(), m_Colourings.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));

  // And finally colour the entire document.
  stc->Colourise(0, stc->GetLength() - 1);
  
  // Todo: should be from wxExSTC.
  const int margin_fold_no = 2;
  
  if (stc->GetProperty("fold") == "1")
  {
    stc->SetMarginWidth(margin_fold_no, 
      wxConfigBase::Get()->ReadLong(_("Folding"), 16));

    stc->SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold Flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  }
  else
  {
    stc->SetMarginWidth(margin_fold_no, 0);
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
    const auto it = 
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
  // accumulate would be nice, but does not add a space, could not do it easily.
  // return accumulate(kset.begin(), kset.end(), wxEmptyString);
  wxString keywords;

  for (
    auto it = kset.begin();
    it != kset.end();
    ++it)
  {
    keywords += *it + " ";
  }

  return keywords.Trim(); // remove the ending space
}

bool wxExLexer::IsKeyword(const wxString& word) const
{
  const auto it = m_Keywords.find(word);
  return (it != m_Keywords.end());
}

bool wxExLexer::IsOk() const
{
  // At this moment ok if scintilla lexer has been filled.
  return !m_ScintillaLexer.empty();
}

bool wxExLexer::KeywordStartsWith(const wxString& word) const
{
  const auto it = m_Keywords.lower_bound(word.Lower());
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
      const auto fill_chars = UsableCharactersPerLine() - text.size();

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

const std::vector<wxExStyle> wxExLexer::ParseNodeColourings(
  const wxXmlNode* node) const
{
  std::vector<wxExStyle> text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "colouring")
    {
      text.push_back(wxExStyle(child));
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined colourings tag: %s on line: %d"),
        child->GetName().c_str(), 
        child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexer::Set(const wxXmlNode* node)
{
  m_ScintillaLexer = node->GetAttribute("name", "");

#ifdef __WXMSW__
  m_Extensions = node->GetAttribute(
    "extensions", 
    "*." + m_ScintillaLexer);
#else    
  m_Extensions = node->GetAttribute("extensions", "");
#endif    

  if (node->GetAttribute("match", "") != "")
  {
    m_Colourings = AutoMatch(node->GetAttribute("match", ""));
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
      m_Colourings = ParseNodeColourings(child);
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
      m_Properties = wxExLexers::Get()->ParseNodeProperties(child);
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

  if (!IsOk())
  {
    wxLogError(_("Illegal lexer on line: %d"), node->GetLineNumber());
  }
}

// Adds the specified keywords to the keywords map and the keywords set.
// The text might contain the keyword set after a ':'.
// Returns true if keyword could be added 
// and false if specified set is illegal.
// Empties existing keywords.
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

      const auto new_setno = atoi(fields.GetNextToken().c_str());

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

bool wxExLexer::SetScintillaLexer(
  const wxString& lexer, 
  wxStyledTextCtrl* stc,
  bool show_error)
{
  stc->ClearDocumentStyle();
  
  for_each (m_Properties.begin(), m_Properties.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExProperty::ApplyReset), stc));

  (*this) = wxExLexers::Get()->FindByName(lexer);
  
  stc->SetLexerLanguage(m_ScintillaLexer);

  if (!IsOk())
  {
    (*this) = wxExLexers::Get()->FindByText(stc->GetLine(0));
    
    stc->SetLexerLanguage(m_ScintillaLexer);
  }

  if (
      IsOk() &&
      // And check whether the GetLexer from scintilla has a good value.
      // Otherwise it is not known, and we better show an error.
      stc->GetLexer() == wxSTC_LEX_NULL &&
      show_error)
  {
    wxLogError(_("Lexer is not known") + ": " + m_ScintillaLexer);
  }

  // Reset keywords, also if no lexer is available.
  for (size_t setno = 0; setno < wxSTC_KEYWORDSET_MAX; setno++)
  {
    stc->SetKeyWords(setno, wxEmptyString);
  }

  // Readme: The Scintilla lexer only recognized lower case words, apparently.
  for (
    auto it = m_KeywordsSet.begin();
    it != m_KeywordsSet.end();
    ++it)
  {
    stc->SetKeyWords(
      it->first,
      GetKeywordsString(it->first).Lower());
  }

  wxExLexers::Get()->GetDefaultStyle().Apply(stc);

  stc->StyleClearAll();

  wxExLexers::Get()->ApplyGlobalStyles(stc);
  wxExLexers::Get()->ApplyIndicators(stc);
  wxExLexers::Get()->ApplyProperties(stc);
  wxExLexers::Get()->ApplyMarkers(stc);
  
  for_each (m_Properties.begin(), m_Properties.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExProperty::Apply), stc));

  Colourise(stc);
  
  return stc->GetLexer() != wxSTC_LEX_NULL;
}

int wxExLexer::UsableCharactersPerLine() const
{
  // We always use lines with 80 characters. We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return 80
    - ((m_CommentBegin.size() != 0) ? m_CommentBegin.size() + 1 : 0)
    - ((m_CommentEnd.size() != 0) ? m_CommentEnd.size() + 1 : 0);
}
