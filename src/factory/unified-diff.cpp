////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::factory::unified_diff
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/factory/frame.h>
#include <wex/factory/unified-diff.h>

#include <iostream>

#define NEXT_TOKEN                                                             \
  if (++tok_iter == tokens.end())                                              \
  {                                                                            \
    return false;                                                              \
  }

#define CHANGES_LINES(RANGE, TEXT)                                             \
  for (int i = 0; i < m_range[RANGE]; i++)                                     \
  {                                                                            \
    NEXT_TOKEN                                                                 \
    m_text[TEXT].push_back((*tok_iter).substr(1));                             \
  }

#define HEADER_LINES(REGEX, INTO)                                              \
  if (!parse_header(REGEX, *tok_iter, INTO))                                   \
  {                                                                            \
    return false;                                                              \
  }                                                                            \
  NEXT_TOKEN

#define SKIP_LINES                                                             \
  while (tok_iter != tokens.end())                                             \
  {                                                                            \
    NEXT_TOKEN                                                                 \
    if (!tok_iter->starts_with("diff ") && !tok_iter->starts_with("index "))   \
    {                                                                          \
      break;                                                                   \
    }                                                                          \
  }

namespace wex
{
size_t stoi(const std::string& i)
{
  return i.empty() ? 1 : std::stoi(i);
}
} // namespace wex

wex::factory::unified_diff::unified_diff(const std::string& input)
  : m_input(input)
{
  m_range.fill({0});
}

bool wex::factory::unified_diff::parse()
{
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

  tokenizer tokens(m_input, boost::char_separator<char>("\r\n"));

  tokenizer::iterator tok_iter = tokens.begin();

  m_diffs = 0;

  while (tok_iter != tokens.end())
  {
    // skip first lines
    SKIP_LINES;

    // The unified output format starts with a two-line header
    HEADER_LINES("--- a/(.*)", m_path[0]);
    HEADER_LINES("\\+\\+\\+ b/(.*)", m_path[1]);

    m_is_first = true;
    m_is_last  = false;

    // Next come one or more chunks of differences
    while (tok_iter != tokens.end())
    {
      if (regex r_chunk("@@ -([0-9]+),?([0-9]*) \\+([0-9]+),?([0-9]*) @@.*");
          r_chunk.match(*tok_iter) != 4)
      {
        log("unified_diff") << *tok_iter << r_chunk.size();
        return false;
      }
      else
      {
        m_range[0] = wex::stoi(r_chunk[0]);
        m_range[1] = wex::stoi(r_chunk[1]);
        m_range[2] = wex::stoi(r_chunk[2]);
        m_range[3] = wex::stoi(r_chunk[3]);
      }

      m_text.fill({});

      // Now get all - lines and all + lines, collect them, and invoke callback.
      CHANGES_LINES(1, 0);
      CHANGES_LINES(3, 1);

      if (!report_diff())
      {
        return false;
      }

      m_is_first = false;

      if (++tok_iter != tokens.end() && !(*tok_iter).starts_with("@@"))
      {
        m_is_last = true;
        report_diff();
        break; // this was last chunk, continue with header lines
      }
    }
  }

  m_is_last = true;

  report_diff();
  report_diff_finish();

  return true;
}

bool wex::factory::unified_diff::parse_header(
  const std::string& r,
  const std::string& line,
  path&              p)
{
  if (regex re(r); !re.match(line))
  {
    log("unified_diff") << line << re.match_data().text();
    return false;
  }
  else
  {
    p = path(re[0]);
  }

  return true;
}
