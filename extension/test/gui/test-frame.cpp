////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExFrame")
{
  GetFrame()->SetFocus(); // otherwise focus is on stc component caused by testEx

  REQUIRE(!GetFrame()->OpenFile(GetTestFile()));
  REQUIRE( ((wxExFrame *)GetFrame())->OpenFile(GetTestFile().GetFullPath(), "contents"));
  
  REQUIRE( GetFrame()->GetGrid() == nullptr);
  REQUIRE( GetFrame()->GetListView() == nullptr);
  REQUIRE( GetFrame()->GetSTC() == nullptr);
  
  GetFrame()->SetFindFocus(nullptr);
  GetFrame()->SetFindFocus(GetFrame());
  GetFrame()->SetFindFocus(GetFrame()->GetSTC());
  
  wxMenuBar* bar = new wxMenuBar();
  GetFrame()->SetMenuBar(bar);
  
  GetFrame()->StatusBarClicked("test");
  GetFrame()->StatusBarClicked("Pane1");
  GetFrame()->StatusBarClicked("Pane2");
  
  GetFrame()->StatusBarClickedRight("test");
  GetFrame()->StatusBarClickedRight("Pane1");
  GetFrame()->StatusBarClickedRight("Pane2");
  
  GetFrame()->SetRecentFile("testing");
  
  REQUIRE(!GetFrame()->StatusText("hello", "test"));
  REQUIRE( GetFrame()->StatusText("hello1", "Pane1"));
  REQUIRE( GetFrame()->StatusText("hello2", "Pane2"));
  REQUIRE( GetFrame()->GetStatusText("Pane1") == "hello1");
  REQUIRE( GetFrame()->GetStatusText("Pane2") == "hello2");
  
  REQUIRE(!GetFrame()->UpdateStatusBar(GetFrame()->GetSTC(), "test"));
  REQUIRE(!GetFrame()->UpdateStatusBar(GetFrame()->GetSTC(), "Pane1"));
  REQUIRE(!GetFrame()->UpdateStatusBar(GetFrame()->GetSTC(), "Pane2"));
  REQUIRE(!GetFrame()->UpdateStatusBar(GetFrame()->GetSTC(), "PaneInfo"));
  
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello stc");
  AddPane(GetFrame(), stc);
  stc->SetFocus();
  
  REQUIRE( GetFrame()->GetSTC() == stc);
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneInfo"));
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneLexer"));
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneFileType"));
  
  for (auto id : std::vector<int> {
    wxID_FIND, wxID_REPLACE, 
    // wxID_OPEN,shows dialog..
    ID_VIEW_MENUBAR, ID_VIEW_STATUSBAR, ID_VIEW_TITLEBAR}) 
  {
    wxPostEvent(GetFrame(), wxCommandEvent(wxEVT_MENU, id));
  }
}
