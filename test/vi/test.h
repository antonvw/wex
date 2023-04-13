////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/vi.h>

void change_mode(
  wex::vi*              vi,
  const std::string&    command,
  wex::vi_mode::state_t mode);
