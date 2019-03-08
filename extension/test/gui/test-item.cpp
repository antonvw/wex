////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wex/item.h>
#include <wex/itemdlg.h>
#include <wex/managedframe.h>
#include "../test-item.h"
#include "test.h"

TEST_CASE("wex::item")
{
  SUBCASE("Item and layout")
  {
    wxPanel* panel = new wxPanel(frame());
    wex::test::add_pane(frame(), panel);
    wxGridSizer* sizer = new wxGridSizer(3);
    panel->SetSizer(sizer);
    
    wex::item::use_config(false);
    
    wex::item item("item", "hello string", 
      wex::item::TEXTCTRL, 
      wex::control_data().is_required(true));
    
    REQUIRE( item.columns() == 1);
    REQUIRE( std::any_cast<std::string>(item.initial()) == "hello string");
    REQUIRE( item.data().is_required());
    REQUIRE( item.label() == "item");
    REQUIRE( item.page().empty());
    REQUIRE( item.type() == wex::item::TEXTCTRL);
    REQUIRE( item.window() == nullptr);
    REQUIRE(!item.get_value().has_value());
    REQUIRE(!item.is_row_growable());
    REQUIRE(!item.apply());
    
    REQUIRE(!item.to_config(false));
    wex::item::use_config(true);
    REQUIRE( item.to_config(false));
    wex::item::use_config(false);
    
    item.set_dialog(nullptr);
    item.set_imagelist(nullptr);
    
    // setting value if window is nullptr should have no effect.
    REQUIRE(!item.set_value("test"));
    REQUIRE(!item.get_value().has_value());
    
    item.set_row_growable(true);
    REQUIRE( item.is_row_growable());
    
    wex::item item_int("int", wex::item::TEXTCTRL_INT, std::string("100"));
    REQUIRE( item_int.type() == wex::item::TEXTCTRL_INT);
    
    wex::item item_int2("int", wex::item::TEXTCTRL_INT, std::string("xxx"));
    REQUIRE( item_int2.type() == wex::item::TEXTCTRL_INT);
    item_int2.layout(panel, sizer);
    REQUIRE( item_int2.window() != nullptr);
    try
    {
      // an excption should be raised as xxx cannot be converted to
      // a long.
      auto val = std::any_cast<long>(item_int2.get_value());
      // therefore, we should not come here
      REQUIRE( 1 == 0);
    }
    catch (std::exception&)
    {
    }
    
    wex::item item_float("float", 
      wex::item::TEXTCTRL_FLOAT, std::string("100.001"));
    REQUIRE( item_float.type() == wex::item::TEXTCTRL_FLOAT);
    
    wex::item item_spin("spindouble", 20.0, 30.0, 25.0, 0.1);
    REQUIRE( item_spin.type() == wex::item::SPINCTRLDOUBLE);

#ifdef __UNIX__
    wex::item item_picker("picker", 
      wex::item::FILEPICKERCTRL, std::string("/usr/bin/git"));
#endif
    
    item.layout(panel, sizer);
    REQUIRE( item.window() != nullptr);
    REQUIRE( std::any_cast<std::string>(item.get_value()) == "hello string");
    REQUIRE( item.set_value(std::string("value changed")));
    REQUIRE( std::any_cast<std::string>(item.get_value()) == "value changed");
    REQUIRE( std::any_cast<std::string>(item.initial()) == "hello string");
    
    REQUIRE( item_int.layout(panel, sizer) != nullptr);
    REQUIRE( item_int.window() != nullptr);
    REQUIRE( std::any_cast<long>(item_int.get_value()) == 100);
    REQUIRE( item_int.set_value(300l));
    REQUIRE( std::any_cast<long>(item_int.get_value()) == 300);

    // Write is tested in wex::item_dialog.
    
    item_float.layout(panel, sizer);
    REQUIRE( std::any_cast<double>(item_float.get_value()) == 100.001);

#ifdef __UNIX__
    REQUIRE( item_picker.layout(panel, sizer) != nullptr);
    REQUIRE( std::any_cast<std::string>(item_picker.get_value()) == 
      "/usr/bin/git");
#endif
    
    std::vector <wex::item> items {item, item_int, item_spin
#ifdef __UNIX__
      , item_picker
#endif
      };
    
    const auto more(test_items());
    items.insert(items.end(), more.begin(), more.end());
    
    // layout the items and check control is created.
    for (auto& it : items)
    {
      // wex::item::USER is not yet laid out ok, gives errors.
      if (it.type() != wex::item::USER)
      {
        // Testing on not nullptr not possible,
        // not all items need a sizer.
        it.layout(panel, sizer);
      }
   
      if (it.type() != wex::item::EMPTY && it.type() != wex::item::SPACER)
      {
        REQUIRE(it.window() != nullptr);
      }
    }
  }

  SUBCASE("Notebooks")
  {
    const std::vector<std::string> titles {
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
    for (
      int style = wex::item::NOTEBOOK; 
      style <= wex::item::NOTEBOOK_WEX;
      style++)
    {
      wxImageList* il = nullptr;
      
      if (style == wex::item::NOTEBOOK_TOOL)
      {
        const wxSize imageSize(32, 32);

        il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
        
        il->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
        il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
      }

      wex::item_dialog* dlg = new wex::item_dialog(
        {test_notebook_item((wex::item::type_t)style, wex::item::LABEL_NONE, il)},
        wex::window_data().
          button(wxOK | wxCANCEL | wxAPPLY).
          title(titles[style - wex::item::NOTEBOOK]));
        
      dlg->Show();

      REQUIRE(std::any_cast<std::string>(dlg->get_item("string1").initial()) == "first");
      REQUIRE(std::any_cast<std::string>(dlg->get_item("string1").get_value()) == "first");
      REQUIRE(dlg->set_item_value("string1", std::string("xxx")));
      REQUIRE(std::any_cast<std::string>(dlg->get_item("string1").get_value()) == "xxx");

      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
      wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
    }
  }
  
  SUBCASE("wex::config_defaults")
  {
    wex::config_defaults def ({
      {"item1", wex::item::TEXTCTRL_INT, 1500l},
      {"item2", wex::item::TEXTCTRL_INT, 1510l}});
  }
}
