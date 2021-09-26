////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <wex/ui/frame.h>
#include <wex/ui/item-dialog.h>

import<vector>;

TEST_SUITE_BEGIN("wex::item");

void check_item_values(wex::item_dialog* dlg, bool default_is_set)
{
  if (default_is_set)
  {
    REQUIRE(
      std::any_cast<std::string>(dlg->get_item_value("stringx")) == "hello1");

    REQUIRE(
      std::any_cast<long>(dlg->get_item_value(std::string("int1"))) == 10l);

    REQUIRE(
      std::any_cast<long>(dlg->get_item_value(std::string("int2"))) == 20l);

    REQUIRE(
      std::any_cast<double>(dlg->get_item_value(std::string("float1"))) ==
      20.0);
  }
  else
  {
    REQUIRE(std::any_cast<std::string>(dlg->get_item_value("stringx")).empty());

    REQUIRE(
      std::any_cast<long>(dlg->get_item_value(std::string("int1"))) == 0l);

    REQUIRE(
      std::any_cast<long>(dlg->get_item_value(std::string("int2"))) == 0l);

    REQUIRE(
      std::any_cast<double>(dlg->get_item_value(std::string("float1"))) == 0);
  }
}

TEST_CASE("wex::item_dialog")
{
  const std::vector<wex::item> items{
    {},
    {"stringx"},
    {"stringy"},
    {"stringz"},
    {"int1", wex::item::TEXTCTRL_INT},
    {"int2", wex::item::TEXTCTRL_INT},
    {"float1", wex::item::TEXTCTRL_FLOAT}};

  const std::vector<wex::item> items_default{
    {"stringx", "hello1"},
    {"stringy", "hello2"},
    {"stringz", "hello3"},
    {"int1", wex::item::TEXTCTRL_INT, std::string("10")},
    {"int2", wex::item::TEXTCTRL_INT, std::string("20")},
    {"float1", wex::item::TEXTCTRL_FLOAT, std::string("20.0")}};

  const std::vector<wex::item> items_empty{};

  SUBCASE("no config")
  {
    wex::item::use_config(false);

    SUBCASE("items")
    {
      auto* dlg = new wex::item_dialog(
        items,
        wex::data::window().button(wxOK | wxCANCEL | wxAPPLY));

      dlg->Show();

      check_item_values(dlg, false);

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
    }

    SUBCASE("items_default")
    {
      auto* dlg = new wex::item_dialog(
        items_default,
        wex::data::window().button(wxOK | wxCANCEL | wxAPPLY));

      dlg->Show();
      dlg->force_checkbox_checked();
      dlg->reload();

#ifdef DEBUG
      for (const auto& i : dlg->get_items())
      {
        std::cout << i.log().str();
      };
#endif

      check_item_values(dlg, true);

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
    }

    SUBCASE("items_empty")
    {
      auto* dlg = new wex::item_dialog(items_empty);
      dlg->Show();
    }
  }

  SUBCASE("config")
  {
    wex::item::use_config(true);

    SUBCASE("items_default")
    {
      auto* dlg = new wex::item_dialog(
        items_default,
        wex::data::window().button(wxOK | wxCANCEL | wxAPPLY));

      dlg->Show();

      check_item_values(dlg, true);

      dlg->reload(true); // save to config

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxCANCEL));
    }

    SUBCASE("items")
    {
      auto* dlg = new wex::item_dialog(
        items,
        wex::data::window().button(wxOK | wxCANCEL | wxAPPLY));

      dlg->Show();

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));

      check_item_values(dlg, true);
    }
  }
}

TEST_SUITE_END();
