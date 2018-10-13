////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/defs.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::frame")
{
  GetSTC()->SetFocus();
  GetSTC()->GetFile().ResetContentsChanged();

  REQUIRE( ((wex::frame *)GetFrame())->OpenFile(GetTestPath("test.h")));
  REQUIRE( ((wex::frame *)GetFrame())->OpenFile(GetTestPath("test.h"), "contents"));
  REQUIRE( ((wex::frame *)GetFrame())->IsOpen(GetTestPath("test.h")));
  REQUIRE(!((wex::frame *)GetFrame())->IsOpen(wex::path("xxx")));
  
  REQUIRE( GetFrame()->GetGrid() == nullptr);
  REQUIRE( GetFrame()->GetListView() == nullptr);
  REQUIRE( GetFrame()->GetSTC() != nullptr);
  GetSTC()->GetFile().ResetContentsChanged();
  
  GetFrame()->SetFindFocus(GetFrame()->GetSTC());
  GetFrame()->SetFindFocus(nullptr);
  GetFrame()->SetFindFocus(GetFrame());
  
  wxMenuBar* bar = new wxMenuBar();
  wex::menu* menu = new wex::menu();
  menu->AppendEdit();
  bar->Append(menu, "Edit");
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
#ifndef __WXOSX__
  REQUIRE( GetFrame()->UpdateStatusBar(GetFrame()->GetSTC(), "PaneInfo"));
#endif
  
  wex::stc* stc = new wex::stc();
  AddPane(GetFrame(), stc);
  stc->SetFocus();
  
  REQUIRE( GetFrame()->GetSTC() == stc);
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneInfo"));
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneLexer"));
  REQUIRE( GetFrame()->UpdateStatusBar(stc, "PaneFileType"));
  
  wxCommandEvent event(wxEVT_MENU, wxID_OPEN);
  stc->GetVi().SetRegisterYank("test.h");
  for (const auto& str : std::vector<wxString> {
    "xxx", "+10 test", "`pwd`", "0"})
  {
    event.SetString(str);
    wxPostEvent(GetFrame(), event);
  }
  
#ifndef __WXMSW__
  for (const auto& id : std::vector<int> {
    wxID_FIND, wxID_REPLACE, 
    wex::ID_VIEW_MENUBAR, wex::ID_VIEW_STATUSBAR, wex::ID_VIEW_TITLEBAR}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(GetFrame(), event);
    wxQueueEvent(GetFrame(), event);
  }
#endif
}
