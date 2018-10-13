////////////////////////////////////////////////////////////////////////////////
// Name:      test-filehistory.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/menu.h>
#include <wx/extension/file.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wex::file_history")
{
  SUBCASE("Default constructor")
  {
    wex::file_history history;
    REQUIRE( history.GetCount() == 0);
    
    wxMenu* menu = new wxMenu();
    menu->Append(1, "x");
    menu->Append(2, "y");

    history.UseMenu(100, menu);
    history.Add("xxx.cpp");
    history.Add("");
    REQUIRE( history.GetCount() == 0);
    REQUIRE( history.GetHistoryFile().Path().empty());
    
    history.Add(GetTestPath("test.h"));
    REQUIRE( history.GetCount() == 1);
    REQUIRE( history.GetHistoryFiles(0).size() == 0);
    REQUIRE( history.GetHistoryFiles(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(GetFrame(), 5);

    history.Clear();
    REQUIRE( history.GetCount() == 0);
    REQUIRE( history.GetHistoryFile().Path().empty());
    REQUIRE( history.GetHistoryFile(100).Path().empty());
    
    history.PopupMenu(GetFrame(), 5);
    history.Save();
  }

  SUBCASE("Other constructor")
  {
    wex::file_history history(4, 1000, "MY-KEY");
    history.Add(GetTestPath("test.h"));
    REQUIRE( history.GetCount() == 1);
    REQUIRE( history.GetBaseId() == 1000);
    REQUIRE( history.GetMaxFiles() == 4);
    history.Save();
  }
  
  SUBCASE("Delete file")
  {
    wex::file_history history;
    history.Clear();
    // file should be closed before remove (at least for windows)
    {
      wex::file file(std::string("test-history.txt"), std::ios_base::out);
      REQUIRE( file.Write(std::string("test")));
    }
    history.Add("test-history.txt");
    REQUIRE( history.GetHistoryFile(0) == "test-history.txt");
    REQUIRE( remove("test-history.txt") == 0);
    REQUIRE( history.GetHistoryFile(0).Path().empty());
  }
}
