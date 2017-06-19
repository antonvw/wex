////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/link.h>
#include <wx/extension/lexer.h>
#include <wx/extension/path.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>

class wxExPaths
{
public:
  wxExPaths() : m_pl(wxExTokenizer(
    wxConfigBase::Get()->Read(_("Include directory")).ToStdString(),
    "\r\n").Tokenize<std::vector<std::string>>()) {;};

  std::string FindPath(const std::string& path) const {
    for (const auto& it : m_pl)
    {
      const wxExPath valid(it, path);

      if (valid.FileExists())
      {
        return valid.GetFullPath();
      }
    }
    return std::string();};
private:
  const std::vector<std::string> m_pl;
};

wxExLink::wxExLink(wxExSTC* stc)
  : m_STC(stc)
  , m_Paths(std::make_unique<wxExPaths>())
{
}

wxExLink::~wxExLink()
{
}

const std::string wxExLink::FindPath(const std::string& text, const wxExControlData& data) const
{
  if (text.empty())
  {
    return std::string();
  }

  // Path in .po files.
  if (
    m_STC != nullptr &&
    m_STC->GetLexer().GetScintillaLexer() == "po" && text.substr(0, 3) == "#: ")
  {
    return text.substr(3);
  }
  
  // hypertext link
  std::vector <std::string> v;
  if (data.Line() < 0 &&
      (wxExMatch("(https?:.*)", text, v) > 0 || 
       wxExMatch("(www.*)", text, v) > 0))
  {
    // with a possible delimiter
    std::string match(v[0]);
    const std::string delimiters("\")");
    
    for (const auto c : delimiters)
    {
      size_t pos = match.find(c);
      
      if (pos != std::string::npos)
      {
        return match.substr(0, pos);
      }
    }
    
    // without delimiter
    return match;
  }
  
  // hypertext file
  if (
    data.Line() == -1 &&
    m_STC != nullptr && 
    m_STC->GetLexer().GetScintillaLexer() == "hypertext")
  {
    return m_STC->GetFileName().GetFullPath();
  }
  
  if (data.Line() < 0) return std::string();

  // Better first try to find "...", then <...>, as in next example.
  // <A HREF="http://www.scintilla.org">scintilla</A> component.

  // So, first get text between " signs.
  size_t pos_char1 = text.find("\"");
  size_t pos_char2 = text.rfind("\"");

  // If that did not succeed, then get text between < and >.
  if (pos_char1 == std::string::npos || 
      pos_char2 == std::string::npos || 
      pos_char2 <= pos_char1)
  {
    pos_char1 = text.find("<");
    pos_char2 = text.rfind(">");
  }

  // If that did not succeed, then get text between ' and '.
  if (pos_char1 == std::string::npos ||
      pos_char2 == std::string::npos || 
      pos_char2 <= pos_char1)
  {
    pos_char1 = text.find("'");
    pos_char2 = text.rfind("'");
  }
  
  const std::string out =
    // If we did not find anything.
     (pos_char1 == std::string::npos || 
      pos_char2 == std::string::npos || 
      pos_char2 <= pos_char1) ?
    text:
    // Okay, get everything inbetween.
    text.substr(pos_char1 + 1, pos_char2 - pos_char1 - 1);

  // And make sure we skip white space.
  return wxExSkipWhiteSpace(out);
}

// text contains selected text, or current line
const std::string wxExLink::GetPath(const std::string& text, wxExControlData& data) const
{
  const std::string path(FindPath(text, data));

  // if http link requested  
  if (data.Line() < 0)
  { 
    return path;
  }
  
  std::string link(path);
  
  SetLink(link, data);
  
  if (link.empty())
  {
    return std::string();
  }

  wxExPath file(link);

  if (file.FileExists())
  {
    return file.MakeAbsolute().GetFullPath();
  }

  if (file.IsRelative() && 
      m_STC != nullptr && m_STC->GetFileName().FileExists())
  {
    wxExPath path(file.MakeAbsolute(m_STC->GetFileName().GetPath()));

    if (path.FileExists())
    {
      return path.GetFullPath();
    }
  }

  // Check whether last word is a file.
  int pos = path.find_last_of(' ');
  std::string word = wxExSkipWhiteSpace((pos != std::string::npos ? path.substr(pos): std::string()));

  if (!word.empty())
  {
    wxExPath path(word);

    if (path.FileExists())
    {
      data.Reset();
      return path.MakeAbsolute().GetFullPath();
    }
  }

  std::string fullpath = m_Paths->FindPath(link);

  if (!fullpath.empty())
  {
    return fullpath;
  }

  if (!word.empty() && SetLink(word, data))
  {
    fullpath = m_Paths->FindPath(word);
  }
  
  return fullpath;
}

bool wxExLink::SetLink(std::string& link, wxExControlData& data) const
{
  if (link.empty())
  {
    return false;
  }

  // Using backslash as separator does not yet work.
  std::replace(link.begin(), link.end(), '\\', '/');

  // The harddrive letter is filtererd, it does not work
  // when adding it to wxExMatch.
  std::string prefix;

#ifdef __WXMSW__
  if (isalpha(link[0]) && link[1] == ':')
  {
    prefix = link.substr(0,1);
    link = link.substr(2);
  }
#endif

  // file[:line[:column]]
  std::vector <std::string> v;
  
  if (wxExMatch("([0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)", link, v) > 0)
  {
    link = v[0];
    data.Reset();
      
    if (v.size() > 1 && !v[1].empty())
    {
      data.Line(std::stoi(v[1]));
        
      if (v.size() > 2 && !v[2].empty())
      {
        data.Col(std::stoi(v[2]));
      }
    }
      
    link = wxExSkipWhiteSpace(prefix + link);
    
    return true;
  }
  
  return false;
}
  
void wxExLink::SetFromConfig()
{
  m_Paths.release();
  m_Paths = std::make_unique<wxExPaths>();
}
