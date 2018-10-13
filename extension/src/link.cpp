////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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

namespace wex
{
  class paths
  {
  public:
    paths() : m_Paths(tokenizer(
      wxConfigBase::Get()->Read(_("Include directory")).ToStdString(),
        "\r\n").Tokenize<std::vector<std::string>>()) {;};

    path FindPath(const std::string& path) const {
      for (const auto& it : m_Paths)
      {
        if (const wex::path valid(it, path); valid.FileExists())
        {
          return valid;
        }
      }
      return wex::path();};
  private:
    const std::vector<std::string> m_Paths;
  };
};

wex::link::link(stc* stc)
  : m_STC(stc)
  , m_Paths(std::make_unique<paths>())
{
}

wex::link::~link()
{
}

const wex::path wex::link::FindPath(
  const std::string& text, const control_data& data) const
{
  if (text.empty() &&
    data.Line() != LINK_LINE_OPEN_MIME &&
    data.Line() != LINK_LINE_OPEN_URL_AND_MIME)
  {
    return wex::path();
  }

  // Path in .po files.
  if (
    m_STC != nullptr &&
    m_STC->GetLexer().GetScintillaLexer() == "po" && text.substr(0, 3) == "#: ")
  {
    return text.substr(3);
  }
  
  // hypertext link
  if (std::vector <std::string> v; 
      (data.Line() == LINK_LINE_OPEN_URL || LINK_LINE_OPEN_URL_AND_MIME) && 
      (match("(https?:.*)", text, v) > 0 || 
       match("(www.*)", text, v) > 0))
  {
    // with a possible delimiter
    const auto match(v[0]);
    const std::string delimiters("\")]");
    
    for (const auto c : delimiters)
    {
      if (const auto pos = match.find(c); pos != std::string::npos)
      {
        return match.substr(0, pos);
      }
    }
    
    // without delimiter
    return match;
  }
  
  if (data.Line() == LINK_LINE_OPEN_URL)
  {
    return std::string();
  }

  // Better first try to find "...", then <...>, as in next example.
  // <A HREF="http://www.scintilla.org">scintilla</A> component.
  for (const auto& p: std::vector < std::pair < std::string, std::string> >
    {{"\"", "\""}, {"<", ">"}, {"[", "]"}, {"'", "'"}, {"{", "}"}})
  {
    const auto pos1 = text.find(p.first);
    const auto pos2 = text.rfind(p.second);

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
    {
      // Okay, get everything inbetween, and make sure we skip white space.
      return skip_white_space(text.substr(pos1 + 1, pos2 - pos1 - 1));
    }
  }

  // Previewable (MIME) file
  if (
    data.Line() == LINK_LINE_OPEN_MIME || 
    data.Line() == LINK_LINE_OPEN_URL_AND_MIME)
  {
    if (m_STC != nullptr && m_STC->GetLexer().Previewable())
    {
      return m_STC->GetFileName();
    }
    else 
    {
      return std::string();
    }
  }
  
  return skip_white_space(text);
}

// text contains selected text, or current line
const wex::path wex::link::GetPath(
  const std::string& text, control_data& data) const
{
  const wex::path path(FindPath(text, data));

  // if http link requested  
  if (data.Line() == LINK_LINE_OPEN_MIME || 
      data.Line() == LINK_LINE_OPEN_URL ||
      data.Line() == LINK_LINE_OPEN_URL_AND_MIME)
  { 
    return path;
  }
  
  wex::path link(path);
  
  SetLink(link, data);
  
  if (link.Path().empty())
  {
    return link;
  }

  wex::path file(link);

  if (file.FileExists())
  {
    return file.MakeAbsolute();
  }

  if (file.IsRelative() && 
      m_STC != nullptr && m_STC->GetFileName().FileExists())
  {
    if (wex::path path(file.MakeAbsolute(m_STC->GetFileName())); path.FileExists())
    {
      return path;
    }
  }

  // Check whether last word is a file.
  const auto pos = path.Path().string().find_last_of(' ');
  wex::path word = skip_white_space((
    pos != std::string::npos ? path.Path().string().substr(pos): std::string()));

  if (!word.Path().empty())
  {
    if (word.FileExists())
    {
      data.Reset();
      return word.MakeAbsolute();
    }
  }

  wex::path fullpath = m_Paths->FindPath(link.Path().string());

  if (!fullpath.Path().empty())
  {
    return fullpath;
  }

  if (!word.Path().empty() && SetLink(word, data))
  {
    fullpath = m_Paths->FindPath(word.Path().string());
  }
  
  return fullpath;
}

bool wex::link::SetLink(path& link, control_data& data) const
{
  if (link.Path().empty())
  {
    return false;
  }

  // The harddrive letter is filtered, it does not work
  // when adding it to match.
  std::string prefix;

#ifdef __WXMSW__
  if (link.Path().string().size() > 1 && 
    isalpha(link.Path().string()[0]) && 
    link.Path().string()[1] == ':')
  {
    prefix = link.Path().string().substr(0, 1);
    link = link.Path().string().substr(2);
  }
#endif

  // file[:line[:column]]
  if (std::vector <std::string> v;
    match("([0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)", link.Path().string(), v) > 0)
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
      
    link = wex::path(skip_white_space(prefix + link.Path().string()));
    
    return true;
  }
  
  return false;
}
  
void wex::link::SetFromConfig()
{
  delete m_Paths.release();
  m_Paths = std::make_unique<wex::paths>();
}
