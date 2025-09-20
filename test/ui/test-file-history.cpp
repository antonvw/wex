////////////////////////////////////////////////////////////////////////////////
// Name:      test-filehistory.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/file.h>
#include <wex/ui/file-history.h>
#include <wex/ui/menu.h>

#include "test.h"

TEST_CASE("wex::file_history")
{
  SECTION("default constructor")
  {
    wex::file_history history;
    history.clear();
    REQUIRE(history.size() == 0);

    auto* menu = new wex::menu({{1, "x"}, {2, "y"}});

    history.use_menu(100, menu);
    REQUIRE(!history.append(wex::path("xxx.cpp")));
    REQUIRE(!history.append(wex::path()));
    REQUIRE(history.size() == 0);
    REQUIRE(history.empty());

    REQUIRE(history.append(wex::test::get_path("test.h")));
    REQUIRE(history.size() == 1);
    REQUIRE(history.get_history_files(0).size() == 0);
    REQUIRE(history.get_history_files(5).size() == 1);

    // next shows a popupmenu, but remains active
    // history.PopupMenu(frame(), 5);

    history.clear();
    REQUIRE(history.empty());
    REQUIRE(history[100].empty());

    history.popup_menu(frame(), 5);
    history.save();
  }

  SECTION("constructor")
  {
    wex::file_history history(4, 1000, "MY-KEY");
    REQUIRE(history.empty());
    REQUIRE(history.append(wex::test::get_path("test.h")));
    REQUIRE(history.size() == 1);
    REQUIRE(history.get_base_id() == 1000);
    REQUIRE(history.get_max_files() == 4);
    history.save();
  }

  SECTION("delete")
  {
    wex::file_history history;
    history.clear();

    // file should be closed before remove (at least for windows)
    {
      wex::file file("test-history.txt", std::ios_base::out);
      REQUIRE(file.write(std::string("test")));
    }

    history.append(wex::path("test-history.txt"));
    REQUIRE(history[0] == wex::path("test-history.txt"));
    REQUIRE(remove("test-history.txt") == 0);
    REQUIRE(history[0].empty()); // this removes the entry
    REQUIRE(history.empty());
  }
}
