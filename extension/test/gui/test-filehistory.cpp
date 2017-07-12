////////////////////////////////////////////////////////////////////////////////
// Name:      test-filehistory.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/menu.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/file.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExFileHistory")
{
  SUBCASE("Default constructor")
  {
    wxExFileHistory history;
    REQUIRE( history.GetCount() == 0);
    
    wxMenu* menu = new wxMenu();
    menu->Append(1, "x");
    menu->Append(2, "y");

    history.UseMenu(100, menu);
    history.AddFileToHistory("xxx.cpp");
    history.AddFileToHistory("");
    REQUIRE( history.GetCount() == 0);
    REQUIRE( history.GetHistoryFile().Path().empty());
    
    history.AddFileToHistory(GetTestFile());
    REQUIRE( history.GetCount() == 1);
    REQUIRE( history.GetHistoryFiles(0).size() == 0);
    REQUIRE( history.GetHistoryFiles(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(GetFrame(), 5);
    // REQUIRE(wxExUIAction(GetFrame()));

    history.Clear();
    REQUIRE( history.GetCount() == 0);
    REQUIRE( history.GetHistoryFile().Path().empty());
    REQUIRE( history.GetHistoryFile(100).Path().empty());
    
    history.PopupMenu(GetFrame(), 5);
    history.Save();
  }

  SUBCASE("Other constructor")
  {
    wxExFileHistory history(4, 1000, "MY-KEY");
    history.AddFileToHistory(GetTestFile());
    REQUIRE( history.GetCount() == 1);
    REQUIRE( history.GetBaseId() == 1000);
    REQUIRE( history.GetMaxFiles() == 4);
    history.Save();
  }
  
  SUBCASE("Delete file")
  {
    wxExFileHistory history;
    history.Clear();
    // file should be closed before remove (at least for windows)
    {
      wxExFile file(std::string("test-history.txt"), wxFile::write);
      REQUIRE( file.Write(std::string("test")));
    }
    history.AddFileToHistory("test-history.txt");
    REQUIRE( history.GetHistoryFile(0) == "test-history.txt");
    REQUIRE( remove("test-history.txt") == 0);
    REQUIRE( history.GetHistoryFile(0).Path().empty());
  }
}
