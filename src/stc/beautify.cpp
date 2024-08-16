////////////////////////////////////////////////////////////////////////////////
// Name:      beautify.cpp
// Purpose:   Implementation of wex::beautify class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/beautify.h>
#include <wex/syntax/path-lexer.h>

wex::beautify::beautify(beautify_t t)
  : factory::beautify(t)
{
}

wex::beautify::beautify(const path& p)
  : factory::beautify(p)
{
}

bool wex::beautify::is_supported(const lexer& l) const
{
  switch (type())
  {
    case SOURCE:
      return l.display_lexer() == "cpp" || l.display_lexer() == "java" ||
             l.display_lexer() == "javascript";

    default:
      return false;
  }
}

bool wex::beautify::stc(wex::stc& s) const
{
  if (!is_active())
  {
    return false;
  }

  const auto& lines(
    s.GetSelectedText().empty() ?
      std::string() :
      " --lines " +
        std::to_string(s.LineFromPosition(s.GetSelectionStart()) + 1) + ":" +
        std::to_string(s.LineFromPosition(s.GetSelectionEnd()) + 1));

  // use the vi escape to use stc text as standard for beautifier,
  // and replace contents with standard output of the process
  return s.get_vi().command(":%!" + name() + lines);
}
