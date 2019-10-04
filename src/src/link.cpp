////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wex/link.h>
#include <wex/config.h>
#include <wex/lexer.h>
#include <wex/path.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

namespace wex
{
  class paths
  {
  public:
    paths() : m_paths(tokenizer(
      config(_("Include directory")).get(), "\r\n").
        tokenize<std::vector<std::string>>()) {;};

    path find_path(const std::string& path) const {
      for (const auto& it : m_paths)
      {
        if (const wex::path valid(it, path); valid.file_exists())
        {
          return valid;
        }
      }
      return wex::path();};
  private:
    const std::vector<std::string> m_paths;
  };
};

wex::link::link(stc* stc)
  : m_stc(stc)
  , m_paths(std::make_unique<paths>())
{
}

wex::link::~link()
{
}

const wex::path wex::link::find_between(const std::string& text) const
{
  if (text.empty())
  {
    return path();
  }

  // Path in .po files.
  if (
    m_stc != nullptr &&
    m_stc->get_lexer().scintilla_lexer() == "po" && text.substr(0, 3) == "#: ")
  {
    return text.substr(3);
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
      return trim(text.substr(pos1 + 1, pos2 - pos1 - 1));
    }
  }

  return trim(text);
}

const wex::path wex::link::find_filename(
  const std::string& text, control_data& data) const
{
  if (text.empty())
  {
    return path();
  }
  
  std::string link(text);
  // The harddrive letter is filtered, it does not work
  // when adding it to match.
  std::string prefix;

#ifdef __WXMSW__
  if (link.size() > 1 && isalpha(link[0]) && link[1] == ':')
  {
    prefix = link.substr(0, 1);
    link = link.substr(2);
  }
#endif

  // file[:line[:column]]
  if (std::vector <std::string> v;
    match("^([0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)", link, v) > 0)
  {
    link = v[0];
    data.reset();
      
    if (v.size() > 1 && !v[1].empty())
    {
      data.line(std::stoi(v[1]));
        
      if (v.size() > 2 && !v[2].empty())
      {
        data.col(std::stoi(v[2]));
      }
    }
      
    path p(trim(prefix + link));
    
    if (const path q(before(p.string(), ':')); q.file_exists())
    {
      return p.make_absolute();
    }
    else
    {
      if (const path r(m_paths->find_path(q.string())); !r.empty())
      {
        return r;
      }
    }
  }
  
  return path();
}

const wex::path wex::link::find_url_or_mime(
  const std::string& text, const control_data& data) const
{
  if (!text.empty())
  {
    // hypertext link
    if (std::vector <std::string> v; 
        (data.line() == LINE_OPEN_URL || LINE_OPEN_URL_AND_MIME) && 
        (match("(https?:.*)", text, v) > 0 || 
         match("(www.*)", text, v) > 0))
    {
      // with a possible delimiter
      const auto match(v[0]);
      const std::string delimiters("\")]");
      
      for (const auto& c : delimiters)
      {
        if (const auto pos = match.find(c); pos != std::string::npos)
        {
          return match.substr(0, pos);
        }
      }
      
      // without delimiter
      return match;
    }
  }
  
  // previewable (MIME) file
  if (m_stc != nullptr && m_stc->get_lexer().previewable())
  {
    return m_stc->get_filename();
  }
  else 
  {
    return path();
  }
}
  
// text contains selected text, or current line
const wex::path wex::link::get_path(
  const std::string& text, control_data& data) const
{
  // mime or url
  if (data.line() == LINE_OPEN_MIME ||
      data.line() == LINE_OPEN_URL ||  
      data.line() == LINE_OPEN_URL_AND_MIME)
  {
    return find_url_or_mime(text, data);
  }

  // if text starts with file:[line[:col]]
  if (const path p(find_filename(text, data)); !p.empty())
  {
    return p;
  }

  // if we have something in between
  const wex::path between(find_between(text));
  
  wex::path link(
    between.is_relative() ? between.string(): trim(text));

  // if between text now starts with file:line:no
  if (const path p(find_filename(link.string(), data)); !p.empty())
  {
    return p;
  }

  // if text now exists, or exists in the stc directory
  if (wex::path file(link); file.file_exists())
  {
    return file.make_absolute();
  }
  else
  {
    if (file.is_relative() && 
        m_stc != nullptr && m_stc->get_filename().file_exists())
    {
      if (wex::path path(m_stc->get_filename().get_path(), file.fullname()); path.file_exists())
      {
        return path;
      }
    }
  }

  // if last word is a file
  const auto pos = between.string().find_last_of(' ');
  wex::path word = trim((
    pos != std::string::npos ? between.string().substr(pos): std::string()));

  if (!word.empty())
  {
    if (word.file_exists())
    {
      data.reset();
      return word.make_absolute();
    }
    
    if (const path p(find_filename(word.string(), data)); !p.empty())
    {
      return p;
    }
  }

  // if text is a file somewhere on the search paths
  return m_paths->find_path(link.string());
}

void wex::link::set_from_config()
{
  m_paths.reset();
  m_paths = std::make_unique<paths>();
}
