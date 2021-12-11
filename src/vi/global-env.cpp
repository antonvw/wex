////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.cpp
// Purpose:   Implementation of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/ex.h>

#include "addressrange-mark.h"
#include "block-lines.h"
#include "global-env.h"

wex::global_env::global_env(const addressrange* ar)
  : m_ex(ar->get_ex())
  , m_ar(ar)
  , m_stc(m_ex->get_stc())
{
  m_stc->set_search_flags(m_ex->search_flags());

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         addressrange::data().commands(),
         boost::char_separator<char>("|")))
  {
    // Prevent recursive global.
    if (it[0] != 'g' && it[0] != 'v')
    {
      m_commands.emplace_back(it);
    }
  }
}

bool wex::global_env::for_each(const block_lines& match) const
{
  return !has_commands() ? m_stc->set_indicator(
                             m_ar->get_find_indicator(),
                             m_stc->GetTargetStart(),
                             m_stc->GetTargetEnd()) :
                           std::all_of(
                             m_commands.begin(),
                             m_commands.end(),
                             [this, match](const std::string& it)
                             {
                               return run(match, it);
                             });
}

// clang-format off
/*
example for global inverse
v/yy/d
line   text  mbs    ibs    ibe   ex action  
0      xx    0      0      13
1      xx    1             2      yy    2      0      2     -> :1,2d2      xx    03      yy    1      0      1     -> :1d      4      yy    2      2      5      yy    3      3      
6      yy    4      4      7      yy    5      5      8      yy    6      6      9      xx    7             10     xx    8    
11     yy    9      7      8     -> :7,8d12     yy    8      8                      13     pp    9                   -> :9d*/// clang-format on
bool wex::global_env::global(const data::substitute& data)
{
  addressrange_mark am(*m_ar);

  if (!am.set())
  {
    log("global could not set marker");
    return false;
  }

  block_lines ib(m_ex, -1, m_stc->get_line_count() - 1);
  block_lines mb(m_ex);

  while (m_stc->SearchInTarget(data.pattern()) != -1)
  {
    mb = am.get_block_lines();

    if (!data.is_inverse())
    {
      if (!process(mb))
      {
        return false;
      }
    }
    else
    {
      if (!process_inverse(mb, ib))
      {
        return false;
      }
    }

    if (!am.update())
    {
      break;
    }
  }

  ib = mb;

  if (ib.end(m_stc->get_line_count() - 1);
      ib.is_available() && data.is_inverse() && !process(ib))
  {
    return false;
  }

  am.end();

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
  if (ib.is_available() && ib < mb)
  {
    if (ib.finish(mb); !process(ib))
    {
      return false;
    }

    ib.reset();
  }
  else
  {
    ib = mb;
  }

  return true;
}

bool wex::global_env::run(const block_lines& block, const std::string& command)
  const
{
  if (const std::string cmd(":" + block.get_range() + command);
      !m_ex->command(cmd))
  {
    m_ex->frame()->show_ex_message(cmd + " failed");
    return false;
  }

  return true;
}
