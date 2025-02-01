////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.cpp
// Purpose:   Implementation of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>
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

bool wex::global_env::command(const block_lines& block, const std::string& text)
  const
{
  if (const auto cmd(":" + block.get_range() + text); !m_ex->command(cmd))
  {
    m_ex->frame()->show_ex_message(cmd + " failed");
    return false;
  }

  return true;
}

bool wex::global_env::for_each(const block_lines& match) const
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
  addressrange_mark am(m_ar, data);

  if (!am.set())
  {
    log("global could not set marker");
    return false;
  }

  const bool infinite =
    (m_recursive && data.commands() != "$" && data.commands() != "1" &&
     data.commands() != "d");

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
      if (!process_inverse(mb, ib))
      {
        return false;
      }
    }
    else
    {
      if (!process(mb))
      {
        return false;
      }
    }

    if (m_hits > 50 && infinite)
    {
      m_ex->frame()->show_ex_message(
        "possible infinite loop at " + mb.get_range());
      return false;
    }

    if (!am.update(m_stc->get_line_count() - lines))
    {
      break;
    }
  }

  if (data.is_inverse())
  {
    ib.start(m_ex->marker_line('T'));
    ib.end(m_ex->marker_line('$') + 1);

    if (ib.is_available() && !process(ib))
    {
      return false;
    }
  }

  am.end(data.is_clear());

  return true;
}

bool wex::global_env::process(const block_lines& block)
{
  block.log();

  if (!for_each(block))
  {
    return false;
  }

  m_hits += block.size();

  return true;
}

bool wex::global_env::process_inverse(const block_lines& mb, block_lines& ib)
{
  // If there is a previous inverse block, process it.
  if (ib < mb)
  {
    if (ib.finish(mb); !process(ib))
    {
      return false;
    }

    ib.start(m_ex->marker_line('T'));
  }
  else
  {
    ib.set_lines(mb);
  }

  return true;
}
