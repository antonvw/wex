////////////////////////////////////////////////////////////////////////////////
// Name:      block-lines.cpp
// Purpose:   Implementation of class wex::block_lines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/syntax/stc.h>

#include "block-lines.h"

wex::block_lines::block_lines(syntax::stc* s, int start, int end, block_t t)
  : m_stc(s)
  , m_start(start)
  , m_end(end)
  , m_type(t)
{
}

void wex::block_lines::end(int line)
{
  m_end = line;
}

void wex::block_lines::finish(const block_lines& block)
{
  m_start++;
  m_end = block.m_start;
}

std::string wex::block_lines::get_range() const
{
  if (!is_available())
  {
    return std::string();
  }

  return size() > 1 ?
           std::to_string(m_start + 1) + "," + std::to_string(m_end) :
           std::to_string(m_start + 1);
}

bool wex::block_lines::is_available() const
{
  return m_start <= m_end;
}

void wex::block_lines::log() const
{
  const std::string name(m_type == block_t::MATCH ? "mb" : "ib");
  log::trace("block_lines " + name) << m_start << "," << m_end;
}

bool wex::block_lines::set_indicator(const indicator& indicator) const
{
  return m_type == block_t::INVERSE ? m_stc->set_indicator(
                                        indicator,
                                        m_stc->PositionFromLine(m_start),
                                        m_stc->PositionFromLine(m_end)) :
                                      m_stc->set_indicator(indicator);
}

size_t wex::block_lines::size() const
{
  return std::max(1, m_end - m_start);
}

void wex::block_lines::start(int start)
{
  m_start = start;
}
