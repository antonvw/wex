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
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExFileHistory")
{
  wxExFileHistory history;
  
  REQUIRE( history.GetCount() == 0);
  
  history.AddFileToHistory("xxx.cpp");
  history.AddFileToHistory("");
  REQUIRE( history.GetCount() == 0);
  REQUIRE( history.GetHistoryFile().empty());
  
  history.AddFileToHistory(GetTestFile().GetFullPath());
  REQUIRE( history.GetCount() == 1);
  REQUIRE( history.GetVector(0).size() == 0);
  REQUIRE( history.GetVector(5).size() == 1);
  
  history.Clear();
  REQUIRE( history.GetCount() == 0);
  REQUIRE( history.GetHistoryFile().empty());
  REQUIRE( history.GetHistoryFile(100).empty());
  
  wxMenu* menu = new wxMenu();
  menu->Append(1, "x");
  menu->Append(2, "y");

  history.PopupMenu(GetFrame(), 5);
  
  history.UseMenu(100, menu);
  
  history.Save();
  
  wxExFileHistory history2(4, 1000, "MY-KEY");
  history2.AddFileToHistory(GetTestFile().GetFullPath());
  REQUIRE( history2.GetCount() == 1);
  history2.Save();
}
