////////////////////////////////////////////////////////////////////////////////
// Name:      log-none.cpp
// Purpose:   Implementation of class wex::log_none
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/core/log.h>

wex::log_none::log_none()
  : m_level(static_cast<int>(log::get_level()))
{
  log::set_level(log::level_t::OFF);
}

wex::log_none::~log_none()
{
  log::set_level((log::level_t)m_level);
}
