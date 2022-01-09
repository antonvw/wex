////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/ui/menu.h>

#include "test.h"

TEST_CASE("wex::menu")
{
  wex::config(_("vcs.Base folder"))
    .set(std::list<std::string>{wxGetCwd().ToStdString()});

  SUBCASE("default constructor")
  {
    auto* menu = new wex::menu;
    REQUIRE(menu->append({{}, {}, {}, {}}) == 0);
  }

  SUBCASE("constructor items")
  {
    auto* menu = new wex::menu(
      {{wxID_SAVE},
       {wex::menu_item::EDIT},
       {wex::menu_item::EDIT_INVERT},
       {wex::menu_item::PRINT},
       {wex::path(), frame(), false},
       {wex::path(wex::path::current()), frame(), false},
       {wxID_SAVE, "mysave"},
       {new wex::menu("submenu", 0), "submenu"}});

    REQUIRE(menu->GetMenuItemCount() > 9);

    auto* menubar = new wxMenuBar;
    menubar->Append(menu, "&Menu");
    frame()->SetMenuBar(menubar);
    frame()->Update();
  }

  SUBCASE("style")
  {
    auto* menu = new wex::menu;
    REQUIRE(menu->style() == wex::menu::menu_t_def());

    menu->style().set(wex::menu::IS_READ_ONLY);
    REQUIRE(menu->style().test(wex::menu::IS_READ_ONLY));
  }
}
