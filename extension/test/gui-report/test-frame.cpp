////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>
#include <wx/extension/frd.h>
#include "test.h"

TEST_CASE("wxExFrameWithHistory")
{
  wxExListView* list = new wxExListView(
    GetFrame(), wxExListView::LIST_HISTORY);

  AddPane(GetFrame(), list);

  wxMenu* menu = new wxMenu();
  GetFrame()->UseFileHistoryList(list);
  GetFrame()->GetProjectHistory().UseMenu(1000, menu);
  list->Show();
  
  REQUIRE(!GetFrame()->OpenFile(GetTestFile())); // as we have no focused stc
  REQUIRE(!GetFrame()->GetFileHistory().GetHistoryFile().Contains("../test.h"));

  REQUIRE(!GetFrame()->OpenFile(
    wxExFileName(GetProject()),
    0,
    wxEmptyString,
    wxExFrameWithHistory::WIN_IS_PROJECT));
  
  wxExFindReplaceData::Get()->SetFindString("wxExTestApp");

  REQUIRE(!GetFrame()->FindInFiles(
    std::vector<wxString> {}, ID_TOOL_REPORT_FIND, false));

  REQUIRE(GetFrame()->FindInFiles(
    std::vector<wxString> {GetTestFile().GetFullPath()}, ID_TOOL_REPORT_FIND, false));

  // GetFrame()->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  REQUIRE(!GetFrame()->GetFindInCaption(ID_TOOL_REPORT_FIND).empty());
  
  // It does not open, next should fail.
  REQUIRE(!GetFrame()->GetProjectHistory().GetHistoryFile().Contains(GetProject()));
  
  REQUIRE( GetFrame()->GetProject() == nullptr);

  REQUIRE( GetFrame()->Grep("xxxxxxx"));
  REQUIRE( GetFrame()->Grep("xxxxxxx yyy"));
  REQUIRE( GetFrame()->Grep("xxxxxxx *.cpp ."));

#ifndef __WXMSW__
  REQUIRE(!GetFrame()->Sed("xxxxxxx"));
#endif
  
  GetFrame()->SetRecentProject("xxx.prj");
  REQUIRE( GetFrame()->GetProjectHistory().GetHistoryFile().empty());

  GetFrame()->SetRecentFile(GetTestFile().GetFullPath());
}
