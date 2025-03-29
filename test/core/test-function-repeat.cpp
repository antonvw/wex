////////////////////////////////////////////////////////////////////////////////
// Name:      test-function-repeat.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wex/core/function-repeat.h>
#include <wex/test/test.h>
#include <wx/frame.h>

TEST_CASE("wex::function_repeat")
{
  int x = 0;

  auto frame = new wxFrame(nullptr, wxID_ANY, "test");
  frame->Show();

  wex::function_repeat repeat(
    "test",
    frame,
    [&x](wxTimerEvent&)
    {
      x += 1;
    });

  REQUIRE(x == 0);

  SECTION("all")
  {
    REQUIRE(repeat.activate());
    REQUIRE(!repeat.activate());

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    wxTheApp->ProcessPendingEvents();

    // REQUIRE(x == 10);

    REQUIRE(repeat.activate(false));
    REQUIRE(!repeat.activate(false));
  }

  SECTION("no-activate")
  {
    REQUIRE(!repeat.activate(false));
  }
}
