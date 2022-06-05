////////////////////////////////////////////////////////////////////////////////
// Name:      test-bind.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/factory/frd.h>

#include "test.h"

TEST_CASE("wex::bind")
{
  wex::factory::find_replace_data frd;

  auto* stc = new wex::test::stc();
  stc->set_text("TM and MvdP are great");

  wex::bind bind(stc);

  bind.command(
    {{[&, stc](wxCommandEvent& event)
      {
        stc->Copy();
      },
      wxID_COPY}});

  bind.ui(
    {{[&, stc](wxUpdateUIEvent& event)
      {
        event.Check(!stc->GetSelectionEmpty());
      },
      wxID_COPY}});

  bind.frd(
    frd.data(),
    [&, stc](const std::string& s, bool b)
    {
      stc->find(s, -1, b);
    });
    
  stc->SelectAll();
  wxCommandEvent event(wxEVT_MENU, wxID_COPY);
  wxPostEvent(stc, event);
  wxTheApp->ProcessPendingEvents();
  
  REQUIRE(wex::clipboard_get() == "TM and MvdP are great");
}
