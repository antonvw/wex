////////////////////////////////////////////////////////////////////////////////
// Name:      test-toolbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/ui/toolbar.h>

#include "test.h"

TEST_CASE("wex::toolbar")
{
  SECTION("controls")
  {
    frame()->get_toolbar()->add_standard(false);
    frame()->get_toolbar()->add_tool(
      {{wxID_CLEAR}, {wxID_FIND}, {wxID_PREFERENCES}, {wex::ID_CLEAR_FILES}});
    frame()->get_toolbar()->Realize();

    frame()->get_find_toolbar()->add_find();

    frame()->get_options_toolbar()->add_checkboxes_standard(false);
    frame()->get_options_toolbar()->add_checkboxes(
      {{wxWindowBase::NewControlId(),
        "ONE",
        "ONE",
        "",
        "this is button 1",
        true,
        [](wxCheckBox*)
        {
          ;
        }},
       {wxWindowBase::NewControlId(),
        "TWO",
        "TWO",
        "",
        "this is button 2",
        false,
        nullptr},
       {wxWindowBase::NewControlId(),
        "THREE",
        "THREE",
        "",
        "this is button 3",
        true,
        nullptr}});

    frame()->pane_show("FINDBAR");
    frame()->pane_show("OPTIONSBAR");
  }

  SECTION("controls")
  {
    REQUIRE(!frame()->get_options_toolbar()->set_checkbox("XX", true));
    REQUIRE(frame()->get_options_toolbar()->set_checkbox("ONE", false));
  }

  SECTION("events")
  {
    // Send events to the find toolbar.
    wxKeyEvent event(wxEVT_CHAR);

    for (auto key : std::vector<
           int>{WXK_UP, WXK_DOWN, WXK_HOME, WXK_END, WXK_PAGEUP, WXK_PAGEDOWN})
    {
      event.m_keyCode = key;
      wxPostEvent(frame()->get_find_toolbar(), event);
    }
  }

  SECTION("hide")
  {
    frame()->pane_show("FINDBAR", false);
    frame()->pane_show("OPTIONSBAR", false);
  }
}
