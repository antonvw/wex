////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-motion.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>

#include "test.h"

void change_mode(
  wex::vi*              vi,
  const std::string&    command,
  wex::vi_mode::state_t mode)
{
  REQUIRE(vi->command(command));
  REQUIRE(vi->mode().get() == mode);
}

std::vector<std::pair<std::string, wex::vi_mode::state_t>> visuals()
{
  return std::vector<std::pair<std::string, wex::vi_mode::state_t>>{
    {"v", wex::vi_mode::state_t::VISUAL},
    {"V", wex::vi_mode::state_t::VISUAL_LINE},
    {"K", wex::vi_mode::state_t::VISUAL_BLOCK}};
}
