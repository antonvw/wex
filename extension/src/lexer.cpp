////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wxExLexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <functional>
//#include <numeric> // both for accumulate
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexer.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExAlignText

// We always use lines with 80 characters. 
const int line_size = 80;

wxExLexer::wxExLexer()
{
  Initialize();
}

wxExLexer::wxExLexer(const wxXmlNode* node)
{
  Initialize();
  Set(node);
}

wxExLexer::wxExLexer(const wxString& lexer, wxStyledTextCtrl* stc, bool clear)
{
  Initialize();
  Set(lexer, stc, true);
}
  
wxExLexer::wxExLexer(const wxExLexer& lexer, wxStyledTextCtrl* stc) 
{
  Initialize();
  Set(lexer, stc);
}
    
// Adds the specified keywords to the keywords map and the keywords set.
// The text might contain the keyword set after a ':'.
// Returns false if specified set is illegal or value is empty.
bool wxExLexer::AddKeywords(const wxString& value)
{
  if (value.empty())
  {
    return false;
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

      if (new_setno <= 0 || new_setno >= wxSTC_KEYWORDSET_MAX)
      {
        wxLogError("Invalid keyword set: %d", new_setno);
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
    }
    else
    {
      keyword = line;
    }

    keywords_set.insert(keyword);
    m_Keywords.insert(keyword);
  }

  auto it = m_KeywordsSet.find(setno);
  
  if (it == m_KeywordsSet.end())
  {
    m_KeywordsSet.insert(make_pair(setno, keywords_set));
  }
  else
  {
    it->second.insert(keywords_set.begin(), keywords_set.end());
  }

  return true;
}

void wxExLexer::Apply(wxStyledTextCtrl* stc, bool clear) const
{
  if (clear)
  {
    stc->ClearDocumentStyle();
  }
  
  for_each (m_Properties.begin(), m_Properties.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExProperty::ApplyReset), stc));

  // Reset keywords, also if no lexer is available.
  for (size_t setno = 0; setno < wxSTC_KEYWORDSET_MAX; setno++)
  {
    stc->SetKeyWords(setno, wxEmptyString);
  }

  if (clear)
  {
    wxExLexers::Get()->ApplyGlobalStyles(stc);
  }

  if (wxExLexers::Get()->GetThemeOk())
  {
    for (const auto& it : m_KeywordsSet)
    {
      stc->SetKeyWords(it.first, GetKeywordsStringSet(it.second));
    }
    
    wxExLexers::Get()->GetDefaultStyle().Apply(stc);
    wxExLexers::Get()->ApplyIndicators(stc);
    wxExLexers::Get()->ApplyProperties(stc);
    wxExLexers::Get()->ApplyMarkers(stc);
  
    for_each (m_Properties.begin(), m_Properties.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExProperty::Apply), stc));

    for_each (m_Styles.begin(), m_Styles.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), stc));
  }

  // And finally colour the entire document.
  stc->Colourise(0, stc->GetLength() - 1);
}

void wxExLexer::ApplyWhenSet(
  const wxString& lexer, wxStyledTextCtrl* stc, bool clear)
{
  if (stc == NULL)
  {
    return;
  }
  
  stc->SetLexerLanguage(m_ScintillaLexer);
  
  if (m_ScintillaLexer.empty() && lexer.empty())
  {
    m_IsOk = (stc->GetLexer() == wxSTC_LEX_NULL);
  }
  else
  {
    m_IsOk = (stc->GetLexer() != wxSTC_LEX_NULL);
  }
  
  if (m_IsOk)
  {
    stc->SetStyleBits(stc->GetStyleBitsNeeded());
    Apply(stc, clear);
      
    if (m_ScintillaLexer == "diff")
    {
      stc->SetEdgeMode(wxSTC_EDGE_NONE);
    }
  }
}

void wxExLexer::AutoMatch(const wxString& lexer)
{
  for (const auto& it : wxExLexers::Get()->GetMacros(lexer))
  {
    const auto& macro = wxExLexers::Get()->GetThemeMacros().find(it.first);

    // First try exact match.
    if (macro != wxExLexers::Get()->GetThemeMacros().end())
    {
      m_Styles.push_back(wxExStyle(it.second, macro->second));
    }
    else
    {
      // Then, a partial using Contains.
      for (const auto& style : wxExLexers::Get()->GetThemeMacros())
      {
        if (it.first.Contains(style.first))
        {
          m_Styles.push_back(wxExStyle(it.second, style.second));
          break;
        }
      }
    }
  }
}

const wxString wxExLexer::CommentComplete(const wxString& comment) const
{
  if (m_CommentEnd.empty())
  {
    return wxEmptyString;
  }
  
  // Fill out rest of comment with spaces, and comment end string.
  const int n = line_size - comment.size() - m_CommentEnd.size();
  
  if (n <= 0)
  {
    return wxEmptyString;
  }
  
  const wxString blanks = wxString(' ', n);
  
  return blanks + m_CommentEnd;
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
    const auto it = m_KeywordsSet.find(keyword_set);

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

  for (const auto& it : kset)
  {
    keywords += it + " ";
  }

  return keywords.Trim(); // remove the ending space
}

void wxExLexer::Initialize()
{
  m_IsOk = false;
  
  m_CommentBegin.clear();
  m_CommentBegin2.clear();
  m_CommentEnd.clear();
  m_CommentEnd2.clear();
  m_Styles.clear();
  m_Extensions.clear();
  m_Properties.clear();
  m_Keywords.clear();
  m_KeywordsSet.clear();
  m_ScintillaLexer.clear();
  m_DisplayLexer.clear();
}

bool wxExLexer::IsKeyword(const wxString& word) const
{
  return m_Keywords.find(word) != m_Keywords.end();
}

bool wxExLexer::KeywordStartsWith(const wxString& word) const
{
  const auto it = m_Keywords.lower_bound(word);
  return 
    it != m_Keywords.end() &&
    it->StartsWith(word);
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

bool wxExLexer::Reset(wxStyledTextCtrl* stc)
{
  Initialize();
  
  stc->SetLexer(wxSTC_LEX_NULL);
  
  m_IsOk = (stc->GetLexer() == wxSTC_LEX_NULL);
  
  if (m_IsOk)
  {
    Apply(stc, true);
  }

  return m_IsOk;
}

bool wxExLexer::Set(
  const wxString& lexer, 
  wxStyledTextCtrl* stc,
  bool clear)
{
  (*this) = wxExLexers::Get()->FindByName(lexer);
  
  if (!m_IsOk && stc != NULL)
  {
    (*this) = wxExLexers::Get()->FindByText(stc->GetLine(0));
  }
    
  ApplyWhenSet(lexer, stc, clear);
  
  if (!m_IsOk)
  {
    wxLogError("Lexer is not known: " + lexer);
  }
  
  return m_IsOk;
}

bool wxExLexer::Set(const wxExLexer& lexer, wxStyledTextCtrl* stc)
{
  if (lexer.GetScintillaLexer().empty() && stc != NULL)
  {
    (*this) = wxExLexers::Get()->FindByText(stc->GetLine(0));
  }
  else
  {
    if (this != &lexer)
    {
      (*this) = lexer;
    }
  }
    
  ApplyWhenSet(lexer.GetScintillaLexer(), stc, true);
  
  return m_IsOk;
}

void wxExLexer::Set(const wxXmlNode* node)
{
  m_ScintillaLexer = node->GetAttribute("name");

  // Just set ok if there is a lexer,
  // when we Apply to a stc component we really can set it.  
  m_IsOk = !m_ScintillaLexer.empty();

  if (!m_IsOk)
  {
    wxLogError("Missing lexer on line: %d", node->GetLineNumber());
  }
  else
  {
    m_DisplayLexer = (!node->GetAttribute("display").empty() ?
      node->GetAttribute("display"):
      m_ScintillaLexer);
    
    m_Extensions = node->GetAttribute("extensions");

    AutoMatch((!node->GetAttribute("macro").empty() ?
      node->GetAttribute("macro"):
      m_ScintillaLexer));

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
      if (child->GetName() == "styles")
      {
        wxExNodeStyles(child, m_ScintillaLexer, m_Styles);
      }
      else if (child->GetName() == "keywords")
      {
        // Add all direct keywords
        const wxString& direct(child->GetNodeContent().Strip(wxString::both));
        
        if (!direct.empty() && !AddKeywords(direct))
        {
          wxLogError(
            "Keywords could not be set on line: %d", 
            child->GetLineNumber());
        }
      
        // Add all keywords that point to a keyword set.
        wxXmlAttribute* att = child->GetAttributes();
        
        while (att != NULL)
        {
          if (!AddKeywords(wxExLexers::Get()->GetKeywords(
            att->GetValue().Strip(wxString::both))))
          {
            wxLogError(
              "Keywords for %s could not be set on line: %d", 
              att->GetValue().c_str(),
              child->GetLineNumber());
          }
        
          att = att->GetNext();
        }
      }
      else if (child->GetName() == "properties")
      {
        wxExNodeProperties(child, m_Properties);
      }
      else if (child->GetName() == "comments")
      {
        m_CommentBegin = child->GetAttribute("begin1");
        m_CommentEnd = child->GetAttribute("end1");
        m_CommentBegin2 = child->GetAttribute("begin2");
        m_CommentEnd2 = child->GetAttribute("end2");
      }
      
      child = child->GetNext();
    }
  }

#ifdef DEBUG
  wxString keywords;
  
  for (const auto& it : m_KeywordsSet)
  {
    keywords += wxString::Format("set: %d %s\n",
      it.first, GetKeywordsString(it.first).c_str());
  }

  if (!keywords.empty())
  {
    wxLogMessage("Lexer: %s Display: %s Keywords: %s",
     m_ScintillaLexer.c_str(), m_DisplayLexer.c_str(), keywords.c_str());
  }
#endif
}

void wxExLexer::SetProperty(const wxString& name, const wxString& value)
{
  for (auto& it : m_Properties)
  {
    if (it.GetName() == name)
    {
      it.Set(value);
      return;
    }
  }

  m_Properties.push_back(wxExProperty(name, value));
}

int wxExLexer::UsableCharactersPerLine() const
{
  // We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return line_size
    - ((m_CommentBegin.size() != 0) ? m_CommentBegin.size() + 1 : 0)
    - ((m_CommentEnd.size() != 0) ? m_CommentEnd.size() + 1 : 0);
}
