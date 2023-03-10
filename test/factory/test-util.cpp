////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wex/factory/frame.h>
#include <wex/factory/util.h>

#include "test.h"

class f_frame : public wex::factory::frame
{
public:
  f_frame()
  {
    Create(nullptr, -1, "frame");
    Show();
  }
};

TEST_CASE("wex::factory::utils")
{
  auto* stc   = new wex::test::stc();
  auto* frame = new f_frame;
  wxTheApp->SetTopWindow(frame);

  REQUIRE(!stc->HasFocus());
  REQUIRE(frame->get_find_focus() == nullptr);

  wex::bind_set_focus(stc);
  REQUIRE(!stc->HasFocus());

  stc->SetFocus();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  REQUIRE(frame->get_find_focus() == stc);
}
