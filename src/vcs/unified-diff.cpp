////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::unified_diff
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/ui/frame.h>
#include <wex/vcs/unified-diff.h>

namespace wex
{
int stoi(const std::string& i)
{
  return i.empty() ? 1 : std::stoi(i);
}
} // namespace wex

wex::unified_diff::unified_diff(const std::string& input, frame* f)
  : m_input(input)
  , m_frame(f)
{
}

void wex::unified_diff::colour(const path& p_from, const path& p_to) const {}

bool wex::unified_diff::parse()
{
  path p_from, p_to;

  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

  tokenizer tokens(m_input, boost::char_separator<char>("\r\n"));

  int line = 0;

  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end();
       ++tok_iter)
  {
    // skip first two lines
    if (line < 2)
    {
      if (++tok_iter == tokens.end())
      {
        log("unified_diff") << line;
        return false;
      }
      
      line++;
      continue;
    }
  
    line++;
    
    // The unified output format starts with a two-line header
    regex r_from_file("--- a/(.*)\t");
    regex r_to_file("\\+\\+\\+ b/(.*)\t");

    if (r_from_file.match(*tok_iter))
    {
      p_from = path(r_from_file[0]);
    }

    if (r_to_file.match(*tok_iter))
    {
      p_to = path(r_to_file[0]);
    }

    // Next come one or more hunks of differences
    regex r_hunk("@@ -([0-9]+),?([0-9]+) +([0-9])+,?([0-9]*) @@");

    if (r_hunk.match(*tok_iter) < 4)
    {
      log("unified_diff") << *tok_iter << r_hunk.size();
      return true;
    }

    m_text[0].clear();
    m_text[1].clear();

    // Now get all - lines and all + lines, collect them, and invoke colour.
    m_range[0] = wex::stoi(r_hunk[0]);
    m_range[1] = wex::stoi(r_hunk[1]);
    m_range[2] = wex::stoi(r_hunk[2]);
    m_range[3] = wex::stoi(r_hunk[3]);

    for (int i = 0; i < m_range[1]; i++)
    {
      if (++tok_iter == tokens.end())
      {
        return false;
      }

      m_text[0].push_back(*tok_iter);
    }

    for (int i = 0; i < m_range[3]; i++)
    {
      if (++tok_iter == tokens.end())
      {
        return false;
      }

      m_text[1].push_back(*tok_iter);
    }

    colour(p_from, p_to);
  }

  return true;
}

bool wex::unified_diff::setup(lexers* l)
{
  return false;
}
