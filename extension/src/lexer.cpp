////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wxExLexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
//#include <numeric> // both for accumulate
//#include <functional>
#include <algorithm>
#include <wx/stc/stc.h>
#include <wx/tokenzr.h>
#include <wx/extension/lexer.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExAlignText

wxExLexer::wxExLexer()
{
  Initialize();
}

wxExLexer::wxExLexer(const wxXmlNode* node)
{
  Initialize();
  
  Set(node);
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

  if (wxExLexers::Get()->GetThemeOk())
  {
    for (
#ifdef wxExUSE_CPP0X	
      auto it = m_KeywordsSet.begin();
#else
      std::map< int, std::set<wxString> >::const_iterator it = m_KeywordsSet.begin();
#endif	  
      it != m_KeywordsSet.end();
      ++it)
    {
      stc->SetKeyWords(
        it->first,
        GetKeywordsString(it->first));
    }
    
    wxExLexers::Get()->GetDefaultStyle().Apply(stc);
  }
  
  if (clear)
  {
    wxExLexers::Get()->ApplyGlobalStyles(stc);
  }

  if (wxExLexers::Get()->GetThemeOk())
  {
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

bool wxExLexer::ApplyLexer(
  const wxString& lexer, 
  wxStyledTextCtrl* stc,
  bool show_error,
  bool clear)
{
  // Even if the lexer is empty.
  (*this) = wxExLexers::Get()->FindByName(lexer);
  
  if (m_ScintillaLexer.empty())
  {
    (*this) = wxExLexers::Get()->FindByText(stc->GetLine(0));
  }
  
  stc->SetLexerLanguage(m_ScintillaLexer);
  
  m_IsOk = 
   (m_ScintillaLexer.empty() && lexer.empty()) ||
   (stc->GetLexer() != wxSTC_LEX_NULL);
  
  if (!m_IsOk && 
      !m_ScintillaLexer.empty()&&
       show_error)
  {
    wxLogError("Lexer is not known: " + m_ScintillaLexer);
  }

  Apply(stc, clear);
  
  return m_IsOk;
}

void wxExLexer::AutoMatch(const wxString& lexer)
{
  for (
    std::map<wxString, wxString>::const_iterator it = 
      wxExLexers::Get()->GetMacros(lexer).begin();
    it != wxExLexers::Get()->GetMacros(lexer).end();
    ++it)
  {
    bool match = false;
    
    for (
#ifdef wxExUSE_CPP0X	
      auto style = wxExLexers::Get()->GetThemeMacros().begin();
#else
      std::map<wxString, wxString>::const_iterator style = wxExLexers::Get()->GetThemeMacros().begin();
#endif	  
      style != wxExLexers::Get()->GetThemeMacros().end() && !match;
      ++style)
    {
      if (it->first.Contains(style->first))
      {
        m_Styles.push_back(wxExStyle(it->second, style->second));
        match = true;
      }
    }
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
#ifdef wxExUSE_CPP0X	
    const auto it = 
      m_KeywordsSet.find(keyword_set);
#else
    const std::map< int, std::set<wxString> >::const_iterator it = 
      m_KeywordsSet.find(keyword_set);
#endif	  

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
#ifdef wxExUSE_CPP0X	
    auto it = kset.begin();
#else
    std::set<wxString>::iterator it = kset.begin();
#endif	
    it != kset.end();
    ++it)
  {
    keywords += *it + " ";
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
#ifdef wxExUSE_CPP0X	
  const auto it = m_Keywords.find(word);
#else
  std::set<wxString>::iterator it = m_Keywords.find(word);
#endif
  
  return (it != m_Keywords.end());
}

bool wxExLexer::KeywordStartsWith(const wxString& word) const
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_Keywords.lower_bound(word);
#else
  std::set<wxString>::iterator it = m_Keywords.lower_bound(word);
#endif
  
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

const std::vector<wxExStyle> wxExLexer::ParseNodeStyles(
  const wxXmlNode* node) const
{
  std::vector<wxExStyle> text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "style")
    {
      text.push_back(wxExStyle(child, m_ScintillaLexer));
    }
    
    child = child->GetNext();
  }

  return text;
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
        const std::vector<wxExStyle> v = ParseNodeStyles(child);
      
        // Do not assign styles to result of ParseNode,
        // as styles might already be filled with result of automatch.
        m_Styles.insert(
          m_Styles.end(), 
          v.begin(), v.end());
      }
      else if (child->GetName() == "keywords")
      {
        if (!SetKeywords(child->GetNodeContent().Strip(wxString::both)))
        {
          wxLogError(
            "Keywords could not be set on line: %d", 
            child->GetLineNumber());
        }
      }
      else if (child->GetName() == "properties")
      {
        m_Properties = wxExLexers::Get()->ParseNodeProperties(child);
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

void wxExLexer::SetProperty(const wxString& name, const wxString& value)
{
  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_Properties.begin();
#else
    std::vector<wxExProperty>::iterator it = m_Properties.begin();
#endif	
    it != m_Properties.end();
    ++it)
  {
    if (it->GetName() == name)
    {
      it->Set(value);
      return;
    }
  }

  m_Properties.push_back(wxExProperty(name, value));
}

int wxExLexer::UsableCharactersPerLine() const
{
  // We always use lines with 80 characters. We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return 80
    - ((m_CommentBegin.size() != 0) ? m_CommentBegin.size() + 1 : 0)
    - ((m_CommentEnd.size() != 0) ? m_CommentEnd.size() + 1 : 0);
}
