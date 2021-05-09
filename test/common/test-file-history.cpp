////////////////////////////////////////////////////////////////////////////////
// Name:      test-filehistory.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/file-history.h>
#include <wex/file.h>
#include <wex/menu.h>

#include "test.h"

TEST_CASE("wex::file_history")
{
  SUBCASE("default constructor")
  {
    wex::file_history history;
    history.clear();
    REQUIRE(history.size() == 0);

    auto* menu = new wex::menu({{1, "x"}, {2, "y"}});

    history.use_menu(100, menu);
    REQUIRE(!history.append(wex::path("xxx.cpp")));
    REQUIRE(!history.append(wex::path()));
    REQUIRE(history.size() == 0);
    REQUIRE(history.path().empty());

    REQUIRE(history.append(wex::test::get_path("test.h")));
    REQUIRE(history.size() == 1);
    REQUIRE(history.get_history_files(0).size() == 0);
    REQUIRE(history.get_history_files(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(get_frame(), 5);

    history.clear();
    REQUIRE(history.size() == 0);
    REQUIRE(history.path().empty());
    REQUIRE(history.path(100).empty());

    history.popup_menu(get_frame(), 5);
    history.save();
  }

  SUBCASE("constructor")
  {
    wex::file_history history(4, 1000, "MY-KEY");
    REQUIRE(history.size() == 0);
    REQUIRE(history.append(wex::test::get_path("test.h")));
    REQUIRE(history.size() == 1);
    REQUIRE(history.get_base_id() == 1000);
    REQUIRE(history.get_max_files() == 4);
    history.save();
  }

  SUBCASE("delete file")
  {
    wex::file_history history;
    history.clear();

    // file should be closed before remove (at least for windows)
    {
      wex::file file("test-history.txt", std::ios_base::out);
      REQUIRE(file.write(std::string("test")));
    }

    history.append(wex::path("test-history.txt"));
    REQUIRE(history.path(0) == wex::path("test-history.txt"));
    REQUIRE(remove("test-history.txt") == 0);
    REQUIRE(history.path(0).empty());
  }
}
