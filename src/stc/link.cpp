////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <wex/config.h>
#include <wex/item-vector.h>
#include <wex/lexer.h>
#include <wex/link.h>
#include <wex/path.h>
#include <wex/regex.h>
#include <wex/stc.h>

namespace wex
{
  class paths
  {
  public:
    paths()
      : m_paths(
          config(_("stc.link.Include directory")).get(std::list<std::string>()))
    {
      ;
    };

    path find(const std::string& path) const
    {
      for (const auto& it : m_paths)
      {
        if (const wex::path valid(it, path); valid.file_exists())
        {
          return valid;
        }
      }

      return wex::path();
    };

  private:
    const std::list<std::string> m_paths;
  };
}; // namespace wex

wex::link::link()
  : m_paths(std::make_unique<paths>())
{
}

wex::link::~link() {}

void wex::link::config_get()
{
  m_paths.reset();
  m_paths = std::make_unique<paths>();
}

const wex::path wex::link::find_between(const std::string& text, stc* stc) const
{
  if (text.empty())
  {
    return path();
  }

  // Path in po files.
  if (regex v("#: "); stc != nullptr &&
                      stc->get_lexer().scintilla_lexer() == "po" &&
                      v.search(text) > 0)
  {
    return v[0];
  }

  // Path in bash files.
  if (regex v("^source (.*)"); stc != nullptr &&
                               stc->get_lexer().scintilla_lexer() == "bash" &&
                               v.search(text) > 0)
  {
    return v[0];
  }

  for (const auto& p : item_vector(stc::config_items())
                         .find<std::list<std::string>>(_("stc.link.Pairs")))
  {
    const auto pos1 = text.find(before(p, '\t'));
    const auto pos2 = text.rfind(after(p, '\t'));

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
    {
      // Okay, get everything in between, and make sure we skip white space.
      return boost::algorithm::trim_copy(
        text.substr(pos1 + 1, pos2 - pos1 - 1));
    }
  }

  return boost::algorithm::trim_copy(text);
}

const wex::path
wex::link::find_filename(const std::string& text, data::control& data) const
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
    link   = link.substr(2);
  }
#endif

  // file[:line[:column]]
  if (regex v("^([0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)"); v.search(link) > 0)
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

    path p(boost::algorithm::trim_copy(prefix + link));

    if (const path q(before(p.string(), ':')); q.file_exists())
    {
      return p.make_absolute();
    }
    else if (const path r(m_paths->find(q.string())); !r.empty())
    {
      return r;
    }
  }

  return path();
}

const wex::path wex::link::find_url_or_mime(
  const std::string&   text,
  const data::control& data,
  stc*                 stc) const
{
  if (!text.empty())
  {
    // hypertext link
    if (regex v({{"(https?:.*)"}, {"(www.*)"}});
        data.line() == LINE_OPEN_URL && v.search(text) > 0)
    {
      // with a possible delimiter
      const auto        match(v[0]);
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
  if (stc != nullptr && stc->get_lexer().is_previewable())
  {
    return stc->get_filename();
  }
  else
  {
    return path();
  }
}

// text contains selected text, or current line
const wex::path wex::link::get_path(
  const std::string& text,
  data::control&     data,
  stc*               stc) const
{
  // mime or url
  if (data.line() == LINE_OPEN_MIME || data.line() == LINE_OPEN_URL)
  {
    return find_url_or_mime(text, data, stc);
  }

  // if text starts with file:[line[:col]]
  if (const path p(find_filename(text, data)); !p.empty())
  {
    return p;
  }

  // if we have something in between
  const wex::path between(find_between(text, stc));

  // if between text now starts with file:line:no
  if (const path p(find_filename(between.string(), data)); !p.empty())
  {
    return p;
  }

  // if text is a file somewhere on the search paths
  if (const auto& p(m_paths->find(between.string())); !p.empty())
  {
    return p;
  }

  // if text now exists, or exists in the stc directory
  if (wex::path file(between); file.file_exists())
  {
    return file.make_absolute();
  }
  else if (
    file.is_relative() && stc != nullptr && stc->get_filename().file_exists())
  {
    if (wex::path path(stc->get_filename().get_path());
        path.append(file).file_exists())
    {
      return path.make_absolute();
    }
  }

  // if last word is a file
  const auto pos = between.string().find_last_of(' ');

  if (wex::path word(boost::algorithm::trim_copy(
        (pos != std::string::npos ? between.string().substr(pos) :
                                    std::string())));
      !word.empty())
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

  return path();
}
