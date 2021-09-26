////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/factory/link.h>
#include <wex/factory/stc.h>
#include <wex/factory/lexer.h>
#include <wex/core/path.h>
#include <wex/core/regex.h>

import<algorithm>;

namespace wex::factory
{
class paths
{
public:
  paths()
    : m_paths(config(_("stc.link.Include directory")).get(config::strings_t()))
  {
    ;
  };

  path find(const std::string& path) const
  {
    for (const auto& it : m_paths)
    {
      if (const wex::path valid(wex::path(it), path); valid.file_exists())
      {
        return valid;
      }
    }

    return wex::path();
  };

private:
  const config::strings_t m_paths;
};
}; // namespace wex::factory

wex::factory::link::link()
  : m_paths(std::make_unique<paths>())
{
}

wex::factory::link::~link() {}

void wex::factory::link::config_get()
{
  m_paths.reset();
  m_paths = std::make_unique<paths>();
}

const wex::path wex::factory::link::find_between(
  const std::string& text,
  factory::stc*      stc) const
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
    return path(v[0]);
  }

  // Path in bash files.
  if (regex v("^source (.*)"); stc != nullptr &&
                               stc->get_lexer().scintilla_lexer() == "bash" &&
                               v.search(text) > 0)
  {
    return path(v[0]);
  }

  if (const auto& lp = get_link_pairs(text); !lp.empty())
  {
    return path(lp);
  }

  return path(boost::algorithm::trim_copy(text));
}

const wex::path wex::factory::link::find_filename(
  const std::string& text,
  line_data&         data) const
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
  if (regex v("^([\\0-9A-Za-z _/.-]+):([0-9]*):?([0-9]*)"); v.search(link) > 0)
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

const wex::path wex::factory::link::find_url_or_mime(
  const std::string& text,
  const line_data&   data,
  factory::stc*      stc) const
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
          return path(match.substr(0, pos));
        }
      }

      // without delimiter
      return path(match);
    }
  }

  // previewable (MIME) file
  if (stc != nullptr && stc->get_lexer().is_previewable())
  {
    return stc->path();
  }
  else
  {
    return path();
  }
}

// text contains selected text, or current line
const wex::path wex::factory::link::get_path(
  const std::string& text,
  line_data&         data,
  factory::stc*      stc) const
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
  else if (file.is_relative() && stc != nullptr && stc->path().file_exists())
  {
    if (wex::path path(stc->path().parent_path());
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
