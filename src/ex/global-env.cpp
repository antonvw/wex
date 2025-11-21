////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.cpp
// Purpose:   Implementation of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frd.h>
#include <wex/ui/frame.h>

#include "addressrange-mark.h"
#include "block-lines.h"
#include "global-env.h"

namespace wex
{
wex::block_lines get_block_lines(syntax::stc* s)
{
  const auto target_start(s->LineFromPosition(s->GetTargetStart()));
  return block_lines(s, target_start, target_start);
}
} // namespace wex

wex::global_env::global_env(const addressrange& ar)
  : m_ex(ar.get_ex())
  , m_ar(ar)
  , m_stc(ar.get_ex()->get_stc())
{
  m_stc->set_search_flags(m_ex->search_flags());

  bool command_arg = false;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         addressrange::data().commands(),
         boost::char_separator<char>("|")))
  {
    // Prevent recursive global.
    if (it[0] != 'g' && it[0] != 'v')
    {
      if (it[0] == 'd' || it[0] == 'm')
      {
        m_recursive = true;
      }

      if (!command_arg)
      {
        m_commands.emplace_back(it);
      }
      else
      {
        // for append, change, insert the | is part of the command
        m_commands.back() += "|" + it;
        command_arg = false;
      }

      if (it == "a" || it == "c" || it == "i")
      {
        command_arg = true;
      }
    }
  }

  if (command_arg)
  {
    log("global command") << m_commands.back() << "missing argument";
    m_commands.clear();
  }
}

wex::global_env::~global_env()
{
  if (m_marker != 0)
  {
    m_ex->marker_delete(m_marker);
    m_marker = 0;
  }
}

bool wex::global_env::command(const block_lines& block, const std::string& exe)
{
  if (const auto cmd(":" + block.get_range() + exe); !m_ex->command(cmd))
  {
    m_ex->frame()->show_ex_message(cmd + " failed");
    return false;
  }

  if (m_ex->command_parsed_data().is_global_skip())
  {
    skip(block.get_range() + exe);
  }

  return true;
}

bool wex::global_env::for_each(const block_lines& match)
{
  return !has_commands() ? match.set_indicator(m_ar.find_indicator()) :
                           std::ranges::all_of(
                             m_commands,
                             [this, match](const std::string& it)
                             {
                               return command(match, it);
                             });
}

// clang-format off
/*
example for global inverse and match block / inverse block
v/yy/d
text   mbs    ibs    ibe   ex action
xx0
xx1
yy2    2      0      2     -> :1,2d
xx3
yy4    2      1      2     -> :2d
yy5    2      2
yy6    3      3
yy7    4      4
yy8    5      5
yy9    6      6
xx10
xx11
yy12   9      7      9     -> :8,9d
yy13   8      8
pp14
                           -> :10d
*/
// clang-format on
bool wex::global_env::global(const data::substitute& data)
{
  addressrange_mark am(m_ar, data, true); // global

  if (!am.set())
  {
    log("global could not set marker");
    return false;
  }

  m_lines_skip.clear();

  block_lines ib(
    m_stc,
    m_ar.begin().get_line() - 2,
    0,
    block_lines::block_t::INVERSE);
  block_lines mb(m_stc);

  while (am.search())
  {
    const auto lines(m_stc->get_line_count());

    if (mb.set_lines(get_block_lines(m_stc)); data.is_inverse())
    {
      if (!process_inverse(am, mb, ib))
      {
        return false;
      }
    }
    else
    {
      if (!process(am, mb))
      {
        return false;
      }
    }

    if (!am.update(m_stc->get_line_count() - lines))
    {
      break;
    }
  }

  if (data.is_inverse())
  {
    ib.start(am.marker_target());
    ib.end(am.marker_end() + 1);

    if (ib.is_available() && !process(am, ib))
    {
      return false;
    }
  }

  am.end(data.is_clear());

  if (m_hits > 0 && !data.is_inverse() && data.commands() != "d")
  {
    find_replace_data::get()->set_find_string(data.pattern());
    m_stc->find(data.pattern());
  }

  return true;
}

bool wex::global_env::process(addressrange_mark& am, const block_lines& block)
{
  block.log();

  int line(
    block.start() + (m_ex->command_parsed_data().command() == "m" ||
                         m_ex->command_parsed_data().text().ends_with('$') ?
                       1 :
                       0));
  bool skip = false;

  while (m_lines_skip.contains(line) &&
         line < m_ex->get_stc()->get_line_count())
  {
    if (!am.skip(line))
    {
      return false;
    }

    skip = true;
    line++;
  }

  if (skip)
  {
    return true;
  }

  if (!for_each(block))
  {
    return false;
  }

  m_hits += block.size();

  return true;
}

bool wex::global_env::process_inverse(
  addressrange_mark& am,
  const block_lines& mb,
  block_lines&       ib)
{
  // If there is a previous inverse block, process it.
  if (ib < mb)
  {
    if (ib.finish(mb); !process(am, ib))
    {
      return false;
    }

    ib.start(am.marker_target());
  }
  else
  {
    ib.set_lines(mb);
  }

  return true;
}

void wex::global_env::skip(const std::string& info)
{
  const address a(m_ex, m_ex->command_parsed_data().text());

  if (m_marker == 0)
  {
    m_marker = '[';
    a.marker_add(m_marker);
  }

  if (const auto& line = skip_marker_line(); line)
  {
    m_lines_skip.insert(*line);
    log::debug("skip inserted line")
      << *line << "marker" << m_ex->marker_line(m_marker) << info << "from"
      << m_ex->command_parsed_data().text();
  }
  else
  {
    log::trace("skip ignored line") << a.get_line() << "all skipped";
  }
}

std::optional<int> wex::global_env::skip_marker_line() const
{
  int line = m_ex->marker_line(m_marker);

  if (line < m_ex->get_stc()->get_line_count() - 1)
  {
    line++;
  }

  while (m_lines_skip.contains(line))
  {
    if (m_ex->command_parsed_data().command() == "m")
    {
      if (line > 0)
      {
        line--;
      }
      else
      {
        return std::nullopt;
      }
    }
    else
    {
      if (line < m_ex->get_stc()->get_line_count())
      {
        line++;
      }
      else
      {
        return std::nullopt;
      }
    }
  }

  return line;
}
