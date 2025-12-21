////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff-parser.cpp
// Purpose:   Implementation of unified_diff_parser
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/parser/parser.hpp>
#include <wex/core/log.h>
#include <wex/factory/unified-diff.h>

#include "unified-diff-parser.h"

namespace bp = boost::parser;

wex::factory::unified_diff_parser::unified_diff_parser(unified_diff* diff)
  : m_diff(diff)
{
  m_diff->m_diffs = 0;
  m_diff->m_type  = unified_diff::diff_t::UNKNOWN;
}

bool wex::factory::unified_diff_parser::parse()
{
  auto const action_diff = [this](const auto& ctx)
  {
    const auto tpl    = _attr(ctx);
    m_diff->m_path[0] = wex::path(std::get<0>(tpl));
    m_diff->m_path[1] = wex::path(std::get<1>(tpl));

    log::debug("unified_diff_parser") << "action_diff" << m_diff->m_path[0];

    for (const auto& hunk : std::get<2>(tpl))
    {
      int index = 0;

      for (const auto& number : std::get<0>(hunk))
      {
        if (const auto* val = std::get_if<std::tuple<int, int>>(&number); val)
        {
          m_diff->m_range[index]     = std::get<0>(*val);
          m_diff->m_range[index + 1] = std::get<1>(*val);
        }
        else
        {
          m_diff->m_range[index]     = std::get<int>(number);
          m_diff->m_range[index + 1] = 1;
        }

        index += 2;
      }

      for (const auto& file : std::get<1>(hunk))
      {
        switch (file[0])
        {
          case '+':
            m_diff->m_text[0].push_back(file.substr(1));
            break;
          case '-':
            m_diff->m_text[1].push_back(file.substr(1));
            break;
          case ' ':
            break;
        }
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

  auto const parser_diff_lines =
    bp::lexeme[+(bp::char_ >> +(bp::char_ - bp::eol))];

  auto const parser_hunk =
    bp::lit("@@") >> bp::repeat(2)[bp::int_ >> ',' >> bp::int_ | bp::int_] >>
    bp::lit("@@") >> bp::lexeme[+(bp::char_ - bp::eol)] >> parser_diff_lines;

  auto const parser_diff = bp::lit("--- a/") >> +(bp::char_ - "+++ b/") >>
                           bp::lit("+++ b/") >> +(bp::char_ - "@@") >>
                           +parser_hunk;

  auto const parser_skip = bp::omit[*(bp::char_ - "--- a/")];

  auto const parser_all = parser_skip >> +parser_diff[action_diff];

  if (const auto result = bp::parse(
        m_diff->input(),
        parser_all,
        bp::ws,
        log::get_level() == log::level_t::TRACE ? bp::trace::on :
                                                  bp::trace::off);
      result)
  {
    return true;
  }

  return false;
}
