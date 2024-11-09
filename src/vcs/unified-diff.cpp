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
#include <wex/factory/frame.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs-entry.h>

#include <iostream>

#define NEXT_TOKEN                                                             \
  if (++tok_iter == tokens.end())                                              \
  {                                                                            \
    return std::nullopt;                                                       \
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
    return std::nullopt;                                                       \
  }                                                                            \
  NEXT_TOKEN

#define SKIP_LINES                                                             \
  for (int i = 0; i < 2; i++)                                                  \
  {                                                                            \
    if (++tok_iter == tokens.end())                                            \
    {                                                                          \
      log("unified_diff") << "missing lines";                                  \
      return std::nullopt;                                                     \
    }                                                                          \
  }

namespace wex
{
int stoi(const std::string& i)
{
  return i.empty() ? 1 : std::stoi(i);
}
} // namespace wex

wex::unified_diff::unified_diff(const std::string& input)
  : m_input(input)
{
  m_range.fill({0});
}

wex::unified_diff::unified_diff(const path& p, vcs_entry* e, factory::frame* f)
  : m_path_vcs(p)
  , m_input(e->std_out())
  , m_frame(f)
  , m_vcs_entry(e)
{
  m_range.fill({0});
}

std::optional<int> wex::unified_diff::parse()
{
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;

  tokenizer tokens(m_input, boost::char_separator<char>("\r\n"));

  tokenizer::iterator tok_iter = tokens.begin();

  int diffs = 0;

  while (tok_iter != tokens.end())
  {
    // skip first lines
    SKIP_LINES;

    // The unified output format starts with a two-line header
    HEADER_LINES("--- a/(.*)", m_path[0]);
    HEADER_LINES("\\+\\+\\+ b/(.*)", m_path[1]);

    // Next come one or more hunks of differences
    while (tok_iter != tokens.end())
    {
      if (regex r_hunk("@@ -([0-9]+),?([0-9]*) \\+([0-9]+),?([0-9]*) @@.*");
          r_hunk.match(*tok_iter) != 4)
      {
        log("unified_diff") << *tok_iter << r_hunk.size();
        return std::nullopt;
      }
      else
      {
        m_range[0] = wex::stoi(r_hunk[0]);
        m_range[1] = wex::stoi(r_hunk[1]);
        m_range[2] = wex::stoi(r_hunk[2]);
        m_range[3] = wex::stoi(r_hunk[3]);
      }

      diffs++;

      m_text.fill({});

      // Now get all - lines and all + lines, collect them, and invoke callback.
      CHANGES_LINES(1, 0);
      CHANGES_LINES(3, 1);

      if (m_frame != nullptr && !m_frame->vcs_unified_diff(m_vcs_entry, this))
      {
        return std::nullopt;
      }

      if (++tok_iter != tokens.end() && !(*tok_iter).starts_with("@@"))
      {
        break; // this was last hunk, continue with header lines
      }
    }
  }

  return std::optional<int>{diffs};
}

bool wex::unified_diff::parse_header(
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
