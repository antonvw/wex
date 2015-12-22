////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/statusbar.h>
#include "test.h"

TEST_CASE("wxExStatusBar")
{
  INFO( GetStatusBar()->GetFieldsCount());
  
  REQUIRE( GetStatusBar()->SetStatusText("hello", ""));
  REQUIRE( GetStatusBar()->SetStatusText("hello0", "Pane0"));
  REQUIRE( GetStatusBar()->SetStatusText("hello1", "Pane1"));
  REQUIRE( GetStatusBar()->SetStatusText("hello2", "Pane2"));
  REQUIRE( GetStatusBar()->SetStatusText("hello3", "Pane3"));
  REQUIRE( GetStatusBar()->SetStatusText("hello4", "Pane4"));
  REQUIRE(!GetStatusBar()->SetStatusText("helloxxx", "Panexxx"));
  REQUIRE(!GetStatusBar()->SetStatusText("hello25", "Pane25"));
  REQUIRE( GetStatusBar()->SetStatusText("GoodBye", "LastPane"));

  REQUIRE( GetStatusBar()->GetStatusText("Pane0") == "hello0");
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello0");
  REQUIRE( GetStatusBar()->GetStatusText("Pane1") == "hello1");
  REQUIRE( GetStatusBar()->GetStatusText("Pane2") == "hello2");
  REQUIRE( GetStatusBar()->GetStatusText("Panexxx").empty());
  
  REQUIRE( GetStatusBar()->ShowField("Pane0", false));
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello1");
  REQUIRE(!GetStatusBar()->ShowField("Pane0", false));
  REQUIRE( GetStatusBar()->ShowField("Pane3", false));
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello1");
  REQUIRE( GetStatusBar()->GetStatusText("Pane0").empty());
  REQUIRE( GetStatusBar()->ShowField("Pane0", true));
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello0");
  REQUIRE( GetStatusBar()->GetStatusText("Pane0") == "hello0");
  REQUIRE( GetStatusBar()->ShowField("LastPane", false));
  REQUIRE( GetStatusBar()->GetStatusText("LastPane").empty());
  REQUIRE(!GetStatusBar()->SetStatusText("BackAgain", "LastPane"));
  REQUIRE( GetStatusBar()->ShowField("LastPane", true));
  REQUIRE( GetStatusBar()->GetStatusText("LastPane") == "BackAgain");
}
