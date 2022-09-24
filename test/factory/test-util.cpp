////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/util.h>
#include <wx/window.h>

#include "../test.h"

TEST_CASE("wex::factory::utils")
{
  wex::bind_set_focus(wxTheApp->GetTopWindow());
}
