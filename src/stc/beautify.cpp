////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.cpp
// Purpose:   Implementation of wex::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/addressrange.h>
#include <wex/beautify.h>
#include <wex/config.h>
#include <wex/factory/process.h>
#include <wex/path-lexer.h>

bool wex::beautify::file(const path& p) const
{
  return is_auto() && is_active() && is_supported(path_lexer(p).lexer()) &&
         factory::process().system(
           name() + " -i " + p.string() +
           " --style=file --fallback-style=none") == 0;
}

bool wex::beautify::is_active() const
{
  return !name().empty();
}

bool wex::beautify::is_auto() const
{
  return config(_("stc.Auto beautify")).get(false);
}

bool wex::beautify::is_supported(const lexer& l) const
{
  return l.display_lexer() == "cpp" || l.display_lexer() == "java" ||
         l.display_lexer() == "javascript";
}

std::list<std::string> wex::beautify::list() const
{
  return std::list<std::string>{{""}, {"clang-format"}};
}

const std::string wex::beautify::name() const
{
  return config(_("stc.Beautifier")).get(list()).front();
}

bool wex::beautify::stc(wex::stc& s) const
{
  if (!is_active())
  {
    return false;
  }

  const std::string& lines(
    s.GetSelectedText().empty() ?
      std::string() :
      " --lines " +
        std::to_string(s.LineFromPosition(s.GetSelectionStart()) + 1) + ":" +
        std::to_string(s.LineFromPosition(s.GetSelectionEnd()) + 1));

  return addressrange(&s.get_vi(), "%").escape(name() + lines);
}
