////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff-parser.cpp
// Purpose:   Implementation of unified_diff_parser
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/parser/parser.hpp>
#include <wex/core/log.h>
#include <wex/factory/unified-diff.h>

#include "unified-diff-parser.h"

namespace bp = boost::parser;

wex::factory::unified_diff_parser::unified_diff_parser(unified_diff* diff)
  : m_diff(diff)
{
  m_diff->m_range.fill({0});
  m_diff->m_diffs    = 0;
  m_diff->m_is_first = true;
  m_diff->m_is_last  = true;
  m_diff->m_type     = unified_diff::diff_t::UNKNOWN;
}

bool wex::factory::unified_diff_parser::parse()
{
  auto const action_diff = [this](const auto& ctx)
  {
    const auto tpl    = _attr(ctx);
    m_diff->m_path[0] = wex::path(std::get<0>(tpl));
    m_diff->m_path[1] = wex::path(std::get<1>(tpl));
    m_diff->m_range.fill({0});

    for (const auto& hunk : std::get<2>(tpl))
    {
      int index = 0;

      m_diff->m_is_first = (hunk == *std::get<2>(tpl).begin());
      m_diff->m_is_last  = (hunk == std::get<2>(tpl).back());

      for (const auto& number : std::get<0>(hunk))
      {
        if (const auto* val = std::get_if<std::tuple<int, int>>(&number); val)
        {
          const int range            = std::get<1>(*val);
          m_diff->m_range[index]     = std::abs(std::get<0>(*val));
          m_diff->m_range[index + 1] = range;

          if (range > 0)
          {
            m_diff->m_diffs++;
          }
        }
        else
        {
          m_diff->m_range[index]     = std::abs(std::get<int>(number));
          m_diff->m_range[index + 1] = 1;
          m_diff->m_diffs++;
        }

        index += 2;
      }

      m_diff->m_text.fill({});

      int i = 0;

      for (const auto& line : std::get<1>(hunk))
      {
        log::trace("unified_diff hunk") << i << line;

        auto fix(line);

        if (line.starts_with("\n"))
        {
          fix = fix.substr(1);
        }

        switch (fix[0])
        {
          case '+':
            m_diff->m_text[1].push_back(fix.substr(1));
            break;
          case '-':
            m_diff->m_text[0].push_back(fix.substr(1));
            break;
          case ' ':
            m_diff->m_text[0].push_back(fix.substr(1));
            m_diff->m_text[1].push_back(fix.substr(1));
            break;
          default:
            log("unified_diff unexpected hunk") << i << fix;
        }

        i++;
      }

      m_diff->m_type =
        (m_diff->m_type == unified_diff::diff_t::UNKNOWN ?
           unified_diff::diff_t::FIRST :
           unified_diff::diff_t::OTHER);

      m_diff->report_diff();
      m_diff->trace("found");
    }
  };

  auto const action_eoi = [this](const auto& ctx)
  {
    m_diff->m_type = unified_diff::diff_t::LAST;
    m_diff->report_diff_finish();
    m_diff->trace("finish");
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
    *(bp::char_("-+ ") >
      bp::lexeme[*(bp::char_ - bp::eol - "\n--- a/" - "\n@@" - "\ndiff --")]);

  auto const parser_hunk =
    bp::lit("@@") > bp::repeat(2)[(bp::int_ >> ',' >> bp::int_) | bp::int_] >>
    bp::lit("@@") > bp::omit[bp::lexeme[+(bp::char_ - bp::eol)]] >
    parser_diff_lines;

  auto const parser_diff = bp::lit("--- a/") >> +(bp::char_ - "+++ b/") >>
                           bp::lit("+++ b/") >>
                           +(bp::char_ - "@@" - "diff --") >> +parser_hunk;

  auto const parser_skip =
    bp::omit[bp::lexeme[*(bp::char_ - (bp::eol >> "--- a/"))]];

  auto const parser_all =
    *(parser_skip >> +parser_diff[action_diff]) > bp::eoi[action_eoi];

  const bool res = bp::parse(m_diff->input(), parser_all, bp::ws);

  if (!res)
  {
    log("unified_diff_parsing") << m_diff->input();
    log::status("Unified diff parsing failed");
  }

  return res;
}
