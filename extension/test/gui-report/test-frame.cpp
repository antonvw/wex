////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>
#include <wx/extension/frd.h>
#include <wx/extension/report/defs.h>
#include "test.h"
#include <easylogging++.h>

TEST_CASE("wxExFrameWithHistory")
{
  wxExListView* list = new wxExListView(wxExListViewData().Type(LIST_HISTORY));

  AddPane(GetFrame(), list);

  wxMenu* menu = new wxMenu();
  GetFrame()->UseFileHistoryList(list);
  GetFrame()->GetProjectHistory().UseMenu(1000, menu);
  list->Show();
  
  REQUIRE(!GetFrame()->OpenFile(GetTestPath("test.h"))); // as we have no focused stc
  REQUIRE( GetFrame()->GetFileHistory().
    GetHistoryFile().Path().string().find("../test.h") == std::string::npos);

  REQUIRE(!GetFrame()->OpenFile(
    wxExPath(GetProject()),
    wxExSTCData().Flags(STC_WIN_IS_PROJECT)));
  
  wxExFindReplaceData::Get()->SetFindString("wxExTestApp");

  REQUIRE(!GetFrame()->FindInFiles({}, ID_TOOL_REPORT_FIND, false));

  REQUIRE( GetFrame()->FindInFiles(
    {GetTestPath("test.h").Path().string()}, ID_TOOL_REPORT_FIND, false));

  // GetFrame()->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  REQUIRE(!GetFrame()->GetFindInCaption(ID_TOOL_REPORT_FIND).empty());
  
  // It does not open, next should fail.
  REQUIRE( GetFrame()->GetProjectHistory().
    GetHistoryFile().Path().string().find(GetProject()) == std::string::npos);
  
  REQUIRE( GetFrame()->GetProject() == nullptr);

  VLOG(9) << "pwd: " << wxExPath::Current();

  REQUIRE( GetFrame()->Grep("xxxxxxx *.cpp ./"));
  REQUIRE( GetFrame()->Grep("xxxxxxx yyy"));
  REQUIRE( GetFrame()->Grep("xxxxxxx"));

#ifndef __WXMSW__
  REQUIRE( GetFrame()->Sed("xxxxxxx yyy"));
#endif
  
  GetFrame()->SetRecentProject("xxx.prj");
  REQUIRE( GetFrame()->GetProjectHistory().GetHistoryFile().Path().empty());

  GetFrame()->SetRecentFile(GetTestPath("test.h"));

  for (auto id : std::vector<int> {
    ID_CLEAR_PROJECTS, ID_PROJECT_SAVE, ID_TOOL_REPORT_FIND, ID_TOOL_REPLACE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(GetFrame(), event);
  }
  
  REQUIRE(wxExUIAction(GetFrame()));
}
