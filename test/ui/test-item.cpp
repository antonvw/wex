////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/ui/item-build.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/item.h>
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/numformatter.h>

#include "test-item.h"
#include "test.h"

TEST_CASE("wex::item")
{
  ITEM_START()

  SUBCASE("group")
  {
    auto* dlg = new wex::item_dialog(
      {wex::add_combobox_with_max("combo", "max"), {"text1"}, {"text2"}});

    dlg->Show();
  }

  SUBCASE("label")
  {
    wex::item::use_config(true);
    wex::item item("item-parent.child", "karmeliet");

    REQUIRE(item.layout(panel, sizer) != nullptr);
    REQUIRE(item.label() == "item-parent.child");
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "karmeliet");
    REQUIRE(item.to_config(true));
    REQUIRE(wex::config("item-parent").exists());
    REQUIRE(!wex::config("item-parent").is_child());
    REQUIRE(wex::config("item-parent.child").exists());
    REQUIRE(!wex::config("item-parent.child").is_child());
    REQUIRE(wex::config("item-parent.child").get() == "karmeliet");
  }

  SUBCASE("notebooks")
  {
    const std::vector<std::string> titles{
      "item::NOTEBOOK",
      "item::NOTEBOOK_AUI",
      "item::NOTEBOOK_CHOICE",
      "item::NOTEBOOK_LIST",
      "item::NOTEBOOK_SIMPLE",
      "item::NOTEBOOK_TOOL",
      "item::NOTEBOOK_TREE",
      "item::NOTEBOOK_WEX"};

    REQUIRE(titles.size() == wex::item::NOTEBOOK_WEX - wex::item::NOTEBOOK + 1);

    // Test dialog using notebook with pages.
    for (int style = wex::item::NOTEBOOK; style <= wex::item::NOTEBOOK_WEX;
         style++)
    {
      CAPTURE(titles[style - wex::item::NOTEBOOK]);

      wxImageList* il = nullptr;

      if (style == wex::item::NOTEBOOK_TOOL)
      {
        const wxSize imageSize(32, 32);

        il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());

        il->Add(
          wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
      }

      auto* dlg = new wex::item_dialog(
        {wex::test_item().notebook(
          (wex::item::type_t)style,
          wex::data::item::LABEL_NONE,
          il)},
        wex::data::window()
          .button(wxOK | wxCANCEL | wxAPPLY)
          .title(titles[style - wex::item::NOTEBOOK]));

      dlg->Show();

      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").data().initial()) ==
        "first");
      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").get_value()) ==
        "first");
      REQUIRE(dlg->set_item_value("string1", std::string("xxx")));
      REQUIRE(
        std::any_cast<std::string>(dlg->find("string1").get_value()) == "xxx");

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
    }
  }

  SUBCASE("staticbox")
  {
    auto* dlg = new wex::item_dialog(
      {{{"staticbox", {{"element1"}, {"element2"}, {"element3"}, {"element4"}}}}});

    dlg->Show();
  }

  SUBCASE("validate")
  {
    wex::item item(
      "item",
      "testxxx",
      wex::item::TEXTCTRL,
      wex::data::item(wex::data::control().is_required(true)));

    REQUIRE(item.validate("[a-z]+"));
    REQUIRE(!item.validate("[0-9]+"));
  }
}
