////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/statusbar.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::statusbar")
{
  REQUIRE( GetStatusBar()->GetFieldsCount() >= 10);

  REQUIRE( GetStatusBar()->set_statustext("hello", ""));
  REQUIRE( GetStatusBar()->set_statustext("hello0", "Pane0"));
  REQUIRE( GetStatusBar()->set_statustext("hello1", "Pane1"));
  REQUIRE( GetStatusBar()->set_statustext("hello2", "Pane2"));
  REQUIRE( GetStatusBar()->set_statustext("hello3", "Pane3"));
  REQUIRE( GetStatusBar()->set_statustext("hello4", "Pane4"));
  REQUIRE(!GetStatusBar()->set_statustext("helloxxx", "Panexxx"));
  REQUIRE(!GetStatusBar()->set_statustext("hello25", "Pane25"));
  REQUIRE( GetStatusBar()->set_statustext("GoodBye", "LastPane"));

  REQUIRE( GetStatusBar()->get_statustext("Pane0") == "hello0");
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello0");
  REQUIRE( GetStatusBar()->get_statustext("Pane1") == "hello1");
  REQUIRE( GetStatusBar()->get_statustext("Pane2") == "hello2");
  REQUIRE( GetStatusBar()->get_statustext("Panexxx").empty());
  
  REQUIRE( GetStatusBar()->show_field("Pane0", false));
  REQUIRE( GetStatusBar()->get_field(0).get_name() == "PaneText");
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello1");
  REQUIRE(!GetStatusBar()->show_field("Pane0", false));
  REQUIRE( GetStatusBar()->show_field("Pane3", false));
  REQUIRE( GetStatusBar()->get_field(1).get_name() == "Pane0");
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello1");
  REQUIRE( GetStatusBar()->get_statustext("Pane0").empty());
  REQUIRE( GetStatusBar()->show_field("Pane0", true));
  REQUIRE( ((wxStatusBar*) GetStatusBar())->GetStatusText(1) == "hello0");
  REQUIRE( GetStatusBar()->get_statustext("Pane0") == "hello0");
  REQUIRE( GetStatusBar()->show_field("LastPane", false));
  REQUIRE( GetStatusBar()->get_statustext("LastPane").empty());
  REQUIRE(!GetStatusBar()->set_statustext("BackAgain", "LastPane"));
  REQUIRE( GetStatusBar()->show_field("LastPane", true));
  REQUIRE( GetStatusBar()->get_statustext("LastPane") == "BackAgain");

  wex::statusbar_pane pane1("PaneInfo", 15, "hello");
  REQUIRE( pane1.get_name() == "PaneInfo");
  REQUIRE( pane1.help_text() == "hello");
  REQUIRE( pane1.get_hidden_text().empty());
  REQUIRE( pane1.GetWidth() == 15);
  REQUIRE( pane1.is_shown() );
  pane1.set_hidden_text("hidden");
  REQUIRE( pane1.get_hidden_text() == "hidden");
  
  GetStatusBar()->setup(frame(), {
    pane1,
    {"PaneLexer"},
    {"PaneFileType"},
    {"Pane1"},
    {"Pane2"}});

  REQUIRE( GetStatusBar()->get_field(0).get_name() == "PaneInfo");
  REQUIRE( GetStatusBar()->GetFieldsCount() == 5);
}
