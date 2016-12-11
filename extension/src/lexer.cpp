////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wxExLexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <algorithm>
#include <functional>
#include <numeric>
#include <wx/tokenzr.h>
#include <wx/xml/xml.h>
#include <wx/extension/lexer.h>
#include <wx/extension/frame.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h> // for wxExAlignText

// We always use lines with 80 characters. 
const int line_size = 80;

wxExLexer& wxExLexer::operator=(const wxExLexer& l)
{
  if (this != &l)
  {
    m_CommentBegin = l.m_CommentBegin;
    m_CommentBegin2 = l.m_CommentBegin2;
    m_CommentEnd = l.m_CommentEnd;
    m_CommentEnd2 = l.m_CommentEnd2;
    m_DisplayLexer = l.m_DisplayLexer;
    m_Extensions = l.m_Extensions;
    m_IsOk = l.m_IsOk;
    m_Keywords = l.m_Keywords;
    m_KeywordsSet = l.m_KeywordsSet;
    m_Language = l.m_Language;
    m_Properties = l.m_Properties;
    m_ScintillaLexer = l.m_ScintillaLexer;
    m_Styles = l.m_Styles;
    
    if (m_STC != nullptr && l.m_STC != nullptr)
    {
      m_STC = l.m_STC;
    }
  }

  return *this;
}
  
// Adds the specified keywords to the keywords map and the keywords set.
// The text might contain the keyword set after a ':'.
// Returns false if specified set is illegal or value is empty.
bool wxExLexer::AddKeywords(const std::string& value, int setno)
{
  if (value.empty() || setno < 0 || setno >= wxSTC_KEYWORDSET_MAX)
  {
    return false;
  }
  
  std::set<std::string> keywords_set;

  wxStringTokenizer tkz(value, "\r\n ");

  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken();
    wxStringTokenizer fields(line, ":");
    std::string keyword;

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
          m_KeywordsSet.insert({setno, keywords_set});
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

  const auto& it = m_KeywordsSet.find(setno);
  
  if (it == m_KeywordsSet.end())
  {
    m_KeywordsSet.insert({setno, keywords_set});
  }
  else
  {
    it->second.insert(keywords_set.begin(), keywords_set.end());
  }

  return true;
}

bool wxExLexer::Apply() const
{
  if (m_STC == nullptr) return false;
  
  m_STC->ClearDocumentStyle();
  
  for_each(m_Properties.begin(), m_Properties.end(), 
    std::bind2nd(std::mem_fun_ref(&wxExProperty::ApplyReset), m_STC));

  // Reset keywords, also if no lexer is available.
  for (size_t setno = 0; setno < wxSTC_KEYWORDSET_MAX; setno++)
  {
    m_STC->SetKeyWords(setno, std::string());
  }

  wxExLexers::Get()->ApplyGlobalStyles(m_STC);

  if (wxExLexers::Get()->GetThemeOk())
  {
    for (const auto& it : m_KeywordsSet)
    {
      m_STC->SetKeyWords(it.first, GetKeywordsStringSet(it.second));
    }
    
    wxExLexers::Get()->Apply(m_STC);
  
    for_each(m_Properties.begin(), m_Properties.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExProperty::Apply), m_STC));

    for_each(m_Styles.begin(), m_Styles.end(), 
      std::bind2nd(std::mem_fun_ref(&wxExStyle::Apply), m_STC));
  }

  // And finally colour the entire document.
  m_STC->Colourise(0, m_STC->GetLength() - 1);
  
  return true;
}

bool wxExLexer::ApplyWhenSet()
{
  if (m_STC == nullptr) return false;
  
  m_STC->SetLexerLanguage(m_ScintillaLexer);
  
  if ((m_IsOk = (m_ScintillaLexer.empty() ? true: (((wxStyledTextCtrl *)m_STC)->GetLexer()) != wxSTC_LEX_NULL)))
  {
    m_STC->SetStyleBits(m_STC->GetStyleBitsNeeded());
    Apply();
      
    if (m_ScintillaLexer == "diff")
    {
      m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
    }
    
    wxExFrame::StatusText(GetDisplayLexer(), "PaneLexer");
  }
  
  return m_IsOk;
}

void wxExLexer::AutoMatch(const std::string& lexer)
{
  const wxExLexer& l(wxExLexers::Get()->FindByName(lexer));
  
  if (l.GetScintillaLexer().empty())
  {
    for (const auto& it : wxExLexers::Get()->GetMacros(lexer))
    {
      const auto& macro = wxExLexers::Get()->GetThemeMacros().find(it.first);

      // First try exact match.
      if (macro != wxExLexers::Get()->GetThemeMacros().end())
      {
        m_Styles.emplace_back(wxExStyle(it.second, macro->second));
      }
      else
      {
        // Then, a partial using Contains.
        const auto& style = std::find_if(wxExLexers::Get()->GetThemeMacros().begin(), wxExLexers::Get()->GetThemeMacros().end(), 
          [&](auto const& e) {return it.first.find(e.first) != std::string::npos;});
        if (style != wxExLexers::Get()->GetThemeMacros().end())
          m_Styles.emplace_back(wxExStyle(it.second, style->second));
      }
    }
  }
  else
  {
    // Copy styles and properties, and not keywords,
    // so your derived display lexer can have it's own keywords.
    m_Styles = l.m_Styles;
    m_Properties = l.m_Properties;
    
    m_CommentBegin = l.m_CommentBegin;
    m_CommentBegin2 = l.m_CommentBegin2;
    m_CommentEnd = l.m_CommentEnd;
    m_CommentEnd2 = l.m_CommentEnd2;
  }
}

const std::string wxExLexer::CommentComplete(const std::string& comment) const
{
  if (m_CommentEnd.empty()) return std::string();
  
  // Fill out rest of comment with spaces, and comment end string.
  const int n = line_size - comment.size() - m_CommentEnd.size();
  
  if (n <= 0) return std::string();
  
  const std::string blanks = std::string(n, ' ');
  
  return blanks + m_CommentEnd;
}

const std::string wxExLexer::GetFormattedText(
  const std::string& lines,
  const std::string& header,
  bool fill_out_with_space,
  bool fill_out) const
{
  std::string text = lines, header_to_use = header;
  size_t nCharIndex;
  std::string out;

  // Process text between the carriage return line feeds.
  while ((nCharIndex = text.find("\n")) != std::string::npos)
  {
    out += wxExAlignText(
      text.substr(0, nCharIndex),
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);

    text = text.substr(nCharIndex + 1);
    header_to_use = std::string(header.size(), ' ');
  }

  if (!text.empty())
  {
    out += wxExAlignText(
      text,
      header_to_use,
      fill_out_with_space,
      fill_out,
      *this);
  }

  return out;
}

const std::string wxExLexer::GetKeywordsString(
  int keyword_set, size_t min_size, const std::string& prefix) const
{
  if (keyword_set == -1)
  {
    return GetKeywordsStringSet(m_Keywords, min_size, prefix);
  }
  else
  {
    const auto& it = m_KeywordsSet.find(keyword_set);

    if (it != m_KeywordsSet.end())
    {
      return GetKeywordsStringSet(it->second, min_size, prefix);
    }
  }

  return std::string();
}

const std::string wxExLexer::GetKeywordsStringSet(
  const std::set<std::string>& kset, size_t min_size, const std::string& prefix) const
{
  return std::accumulate(kset.begin(), kset.end(), std::string{}, 
    [&](const std::string& a, const std::string& b) {
      return (b.size() >= min_size && wxString(b).StartsWith(prefix)) ? a + b + ' ': a;});
}

void wxExLexer::Initialize()
{
  m_CommentBegin.clear();
  m_CommentBegin2.clear();
  m_CommentEnd.clear();
  m_CommentEnd2.clear();
  m_DisplayLexer.clear();
  m_Extensions.clear();
  m_IsOk = true;
  m_Keywords.clear();
  m_KeywordsSet.clear();
  m_Properties.clear();
  m_ScintillaLexer.clear();
  m_Styles.clear();
}

bool wxExLexer::IsKeyword(const std::string& word) const
{
  return m_Keywords.find(word) != m_Keywords.end();
}

bool wxExLexer::KeywordStartsWith(const std::string& word) const
{
  const auto& it = m_Keywords.lower_bound(word);
  return 
    it != m_Keywords.end() &&
    it->find(word) == 0;
}

const std::string wxExLexer::MakeComment(
  const std::string& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  std::string out;

  text.find("\n") != std::string::npos ?
    out += GetFormattedText(text, std::string(), fill_out_with_space, fill_out):
    out += wxExAlignText(text, std::string(), fill_out_with_space, fill_out, *this);

  return out;
}

const std::string wxExLexer::MakeComment(
  const std::string& prefix,
  const std::string& text) const
{
  std::string out;

  text.find("\n") != std::string::npos ?
    out += GetFormattedText(text, prefix, true, true):
    out += wxExAlignText(text, prefix, true, true, *this);

  return out;
}

const std::string wxExLexer::MakeSingleLineComment(
  const std::string& text,
  bool fill_out_with_space,
  bool fill_out) const
{
  if (m_CommentBegin.empty() && m_CommentEnd.empty())
  {
    return text;
  }

  // First set the fill_out_character.
  char fill_out_character;

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

  std::string out = m_CommentBegin + fill_out_character + text;

  // Fill out characters.
  if (fill_out)
  {
    // To prevent filling out spaces
    if (fill_out_character != ' ' || !m_CommentEnd.empty())
    {
      const int fill_chars = UsableCharactersPerLine() - text.size();

      if (fill_chars > 0)
      {
        out += std::string(fill_chars, fill_out_character);
      }
    }
  }

  if (!m_CommentEnd.empty()) out += fill_out_character + m_CommentEnd;

  return out;
}

bool wxExLexer::Reset()
{
  Initialize();
  
  if (m_STC != nullptr)
  {
    ((wxStyledTextCtrl *)m_STC)->SetLexer(wxSTC_LEX_NULL);
    
    if ((m_IsOk = (((wxStyledTextCtrl *)m_STC)->GetLexer()) == wxSTC_LEX_NULL))
    {
      Apply();
      wxExFrame::StatusText(GetDisplayLexer(), "PaneLexer");
      m_STC->ResetMargins(STC_MARGIN_FOLDING);
    }
  }

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
    m_Language = node->GetAttribute("language");

    AutoMatch((!node->GetAttribute("macro").empty() ?
      node->GetAttribute("macro").ToStdString():
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
        const std::string& direct(child->GetNodeContent().Strip(wxString::both).ToStdString());
        
        if (!direct.empty() && !AddKeywords(direct))
        {
          wxLogError(
            "Keywords could not be set on line: %d", 
            child->GetLineNumber());
        }
      
        // Add all keywords that point to a keyword set.
        wxXmlAttribute* att = child->GetAttributes();
        
        while (att != nullptr)
        {
          const int setno = atoi(att->GetName().AfterFirst('-'));
          const std::string keywords = wxExLexers::Get()->GetKeywords(
            att->GetValue().Strip(wxString::both).ToStdString());
          
          if (!AddKeywords(keywords, setno))
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
        if (!m_Properties.empty())
        {
          wxLogError(
            "Properties already available on line: %d", 
            child->GetLineNumber());
        }
        
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
}

bool wxExLexer::Set(const std::string& lexer, bool fold)
{
  // If there are no lexers, just return, to prevent error message.
  if (wxExLexers::Get()->GetLexers().empty()) return false;

  (*this) = wxExLexers::Get()->FindByName(lexer);

  if (!m_IsOk && m_STC != nullptr)
  {
    (*this) = wxExLexers::Get()->FindByText(m_STC->GetLine(0).ToStdString());
  }
    
  if (m_ScintillaLexer.empty() && !lexer.empty())
  {
    m_IsOk = false;
  }
  else
  {
    if (ApplyWhenSet() && fold && m_STC != nullptr)
    {
      m_STC->Fold();
    }
  }
  
  if (!m_IsOk)
  {
    wxLogError("Lexer is not known: %s", lexer.c_str());
  }
  
  return m_IsOk;
}

bool wxExLexer::Set(const wxExLexer& lexer, bool fold)
{
  if (lexer.GetScintillaLexer().empty() && m_STC != nullptr)
  {
    (*this) = wxExLexers::Get()->FindByText(m_STC->GetLine(0).ToStdString());
  }
  else
  {
    if (this != &lexer)
    {
      (*this) = lexer;
    }
  }
    
  if (ApplyWhenSet() && fold && m_STC != nullptr)
  {
    m_STC->Fold();
  }

  return m_IsOk;
}

void wxExLexer::SetProperty(const std::string& name, const std::string& value)
{
  const auto& it = std::find_if(m_Properties.begin(), m_Properties.end(), 
    [name](auto const& e) {return e.GetName() == name;});
  if (it != m_Properties.end()) it->Set(value);
  else m_Properties.emplace_back(wxExProperty(name, value));
}

int wxExLexer::UsableCharactersPerLine() const
{
  // We adjust this here for
  // the space the beginning and end of the comment characters occupy.
  return line_size
    - ((m_CommentBegin.size() != 0) ? m_CommentBegin.size() + 1 : 0)
    - ((m_CommentEnd.size() != 0) ? m_CommentEnd.size() + 1 : 0);
}
