////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
  SECTION("Default constructor")
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
    REQUIRE( history.GetHistoryFile().empty());
    
    history.AddFileToHistory(GetTestFile().GetFullPath());
    REQUIRE( history.GetCount() == 1);
    REQUIRE( history.GetVector(0).size() == 0);
    REQUIRE( history.GetVector(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(GetFrame(), 5);
    // REQUIRE(wxExUIAction(GetFrame()));

    history.Clear();
    REQUIRE( history.GetCount() == 0);
    REQUIRE( history.GetHistoryFile().empty());
    REQUIRE( history.GetHistoryFile(100).empty());
    
    history.PopupMenu(GetFrame(), 5);
    history.Save();
  }

  SECTION("Other constructor")
  {
    wxExFileHistory history(4, 1000, "MY-KEY");
    history.AddFileToHistory(GetTestFile().GetFullPath());
    REQUIRE( history.GetCount() == 1);
    history.Save();
  }
  
  SECTION("Delete file")
  {
    wxExFileHistory history;
    history.Clear();
    // file should be closed before remove (at least for windows)
    {
      wxExFile file("test-history.txt", wxFile::write);
      REQUIRE( file.Write(wxString("test")));
    }
    history.AddFileToHistory("test-history.txt");
    REQUIRE( history.GetHistoryFile(0) == "test-history.txt");
    REQUIRE( remove("test-history.txt") == 0);
    REQUIRE( history.GetHistoryFile(0).empty());
  }
}
