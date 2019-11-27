////////////////////////////////////////////////////////////////////////////////
// Name:      test-filehistory.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/file.h>
#include <wex/filehistory.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include "test.h"

TEST_CASE("wex::file_history")
{
  SUBCASE("Default constructor")
  {
    wex::file_history history;
    history.clear();
    REQUIRE( history.size() == 0);
    
    auto* menu = new wex::menu();
    menu->Append(1, "x");
    menu->Append(2, "y");

    history.use_menu(100, menu);
    history.add("xxx.cpp");
    history.add("");
    REQUIRE( history.size() == 0);
    REQUIRE( history.get_history_file().empty());
    
    history.add(wex::test::get_path("test.h"));
    REQUIRE( history.size() == 1);
    REQUIRE( history.get_history_files(0).size() == 0);
    REQUIRE( history.get_history_files(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(frame(), 5);

    history.clear();
    REQUIRE( history.size() == 0);
    REQUIRE( history.get_history_file().empty());
    REQUIRE( history.get_history_file(100).empty());
    
    history.popup_menu(frame(), 5);
    history.save();
  }

  SUBCASE("Other constructor")
  {
    wex::file_history history(4, 1000, "MY-KEY");
    history.add(wex::test::get_path("test.h"));
    REQUIRE( history.size() == 1);
    REQUIRE( history.get_base_id() == 1000);
    REQUIRE( history.get_max_files() == 4);
    history.save();
  }
  
  SUBCASE("Delete file")
  {
    wex::file_history history;
    history.clear();
    // file should be closed before remove (at least for windows)
    {
      wex::file file(std::string("test-history.txt"), std::ios_base::out);
      REQUIRE( file.write(std::string("test")));
    }
    history.add("test-history.txt");
    REQUIRE( history.get_history_file(0) == "test-history.txt");
    REQUIRE( remove("test-history.txt") == 0);
    REQUIRE( history.get_history_file(0).empty());
  }
}
