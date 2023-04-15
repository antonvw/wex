////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/vi.h>

/// Runs command, and verifies vi mode to the specified one.
void change_mode(
  wex::vi*              vi,
  const std::string&    command,
  wex::vi_mode::state_t mode);

/// Returns vector containing commands to change to visual mode.
std::vector<std::pair<std::string, wex::vi_mode::state_t>> visuals();
