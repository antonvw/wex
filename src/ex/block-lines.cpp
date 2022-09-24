////////////////////////////////////////////////////////////////////////////////
// Name:      block-lines.cpp
// Purpose:   Implementation of class wex::block_lines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/ex/ex.h>
#include <wex/syntax/stc.h>

#include "block-lines.h"

wex::block_lines::block_lines(ex* ex, int start, int end)
  : m_ex(ex)
  , m_stc(ex->get_stc())
  , m_start(start)
  , m_end(end)
  , m_name(start == -1 ? "ib" : "mb")
{
}

wex::block_lines& wex::block_lines::operator=(const wex::block_lines& r)
{
  if (this != &r)
  {
    m_end   = r.m_end;
    m_ex    = r.m_ex;
    m_name  = r.m_name;
    m_start = r.m_start;
    m_stc   = r.m_ex->get_stc();
  }

  return *this;
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
  return size() > 1 ?
           std::to_string(m_start + 1) + "," + std::to_string(m_end) :
           std::to_string(m_start + 1);
}

bool wex::block_lines::is_available() const
{
  return m_start != LINE_RESET;
}

void wex::block_lines::log() const
{
  log::trace("block lines " + m_name) << "start:" << m_start << "end:" << m_end;
}

wex::block_lines wex::block_lines::single() const
{
  const int start(m_stc->LineFromPosition(m_stc->GetTargetStart()));
  return {m_ex, start, start};
}

size_t wex::block_lines::size() const
{
  return std::max(1, m_end - m_start);
}

void wex::block_lines::start(int start)
{
  m_start = start;
}

wex::block_lines wex::block_lines::target() const
{
  return {
    m_ex,
    m_stc->LineFromPosition(m_stc->GetTargetStart()),
    m_stc->LineFromPosition(m_stc->GetTargetEnd())};
}
