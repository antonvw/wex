////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff-parser.cpp
// Purpose:   Implementation of unified_diff_parser
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/parser/parser.hpp>
#include <wex/factory/unified-diff.h>

#include "unified-diff-parser.h"

namespace bp = boost::parser;

enum action_t_str
{
  ACTION_DEL,
  ACTION_ADD,
  ACTION_BOTH,
};

struct diff
{
  std::string path_a, path_b;

  std::vector<std::vector<std::variant<std::pair<int, int>, int>>> hunks;

  std::vector<std::string> files;
};

wex::factory::unified_diff_parser::unified_diff_parser(unified_diff* diff)
  : m_diff(diff)
{
  m_diff->m_diffs = 0;
  m_diff->m_type  = unified_diff::diff_t::UNKNOWN;
}

bool wex::factory::unified_diff_parser::parse(bool trace_mode)
{
  bp::symbols<int> const actions = {
    {"-", ACTION_DEL},
    {"+", ACTION_ADD},
    {" ", ACTION_BOTH}};

  auto const action = [this](auto& ctx)
  {
    std::cout << "Got one!\n";
    diff x;

    m_diff->m_path[0] = wex::path(x.path_a);
    m_diff->m_path[1] = wex::path(x.path_b);

    for (const auto& hunk : x.hunks)
    {
      int index = 0;

      for (const auto& number : hunk)
      {
        if (const auto* val = std::get_if<std::pair<int, int>>(&number); val)
        {
          m_diff->m_range[index]     = val->first;
          m_diff->m_range[index + 1] = val->second;
        }
        else
        {
          m_diff->m_range[index]     = std::get<int>(number);
          m_diff->m_range[index + 1] = 1;
        }

        index += 2;
      }

      m_diff->m_type =
        (m_diff->m_type == unified_diff::diff_t::UNKNOWN ?
           unified_diff::diff_t::FIRST :
           unified_diff::diff_t::OTHER);

      m_diff->report_diff();
    }
  };

  // (Skip the first lines)
  // The unified output format starts with a two-line header:
  // --- from-file from-file-modification-time
  // +++ to-file to-file-modification-time
  // Next come one or more hunks of differences:
  // @@ from-file-line-numbers to-file-line-numbers @@
  // line-from-either-file
  // line-from-either-file...

  auto const skip_lines = bp::omit[*(bp::char_ - "--- a/")];

  auto const parser_diff =
    bp::lit("--- a/") >> +(bp::char_ - "+++ b/") >> bp::lit("+++ b/") >>
    +(bp::char_ - "@@") >>
    +(bp::lit("@@") >> bp::repeat(2)[bp::int_ >> ',' >> bp::int_ | bp::int_] >>
      bp::lit("@@")) >>
    *(bp::char_ - bp::eol); // >> bp::eol >>
                            //    +(bp::char_);
                            //    +(actions >> +bp::char_);

  auto const action_parser = skip_lines >> +(parser_diff[action]);

  if (const auto result = bp::parse(
        m_diff->input(),
        action_parser,
        bp::ws,
        trace_mode ? bp::trace::on : bp::trace::off);
      result)
  {
    return true;
  }

  return false;
}
