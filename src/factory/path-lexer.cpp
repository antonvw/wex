////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path_lexer
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lexers.h>
#include <wex/path-lexer.h>

namespace fs = std::filesystem;

namespace wex
{
  const std::string lexer_string(const std::string& filename)
  {
    lexers* l = lexers::get(false);
    return l != nullptr && !l->get_lexers().empty() ?
             l->find_by_filename(filename).display_lexer() :
             std::string();
  }
}; // namespace wex

wex::path_lexer::path_lexer(const std::string& p)
  : path(p)
  , m_lexer(lexer_string(p))
{
}

wex::path_lexer::path_lexer(const path& p)
  : path(p)
  , m_lexer(lexer_string(p.string()))
{
}
