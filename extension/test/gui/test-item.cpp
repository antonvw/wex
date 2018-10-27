////////////////////////////////////////////////////////////////////////////////
// Name:      test-Item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
  SUBCASE("Item and Layout")
  {
    wxPanel* panel = new wxPanel(GetFrame());
    AddPane(GetFrame(), panel);
    wxGridSizer* sizer = new wxGridSizer(3);
    panel->SetSizer(sizer);
    
    wex::item::UseConfig(false);
    
    wex::item item("item", "hello string", wex::item::TEXTCTRL, wex::control_data().Required(true));
    
    REQUIRE( item.GetColumns() == 1);
    REQUIRE( std::any_cast<std::string>(item.GetInitial()) == "hello string");
    REQUIRE( item.GetData().Required());
    REQUIRE( item.GetLabel() == "item");
    REQUIRE( item.GetPage().empty());
    REQUIRE( item.GetType() == wex::item::TEXTCTRL);
    REQUIRE( item.GetWindow() == nullptr);
    REQUIRE(!item.GetValue().has_value());
    REQUIRE(!item.IsRowGrowable());
    REQUIRE(!item.Apply());
    
    REQUIRE(!item.ToConfig(false));
    wex::item::UseConfig(true);
    REQUIRE( item.ToConfig(false));
    wex::item::UseConfig(false);
    
    item.SetDialog(nullptr);
    item.SetImageList(nullptr);
    
    // setting value if window is nullptr should have no effect.
    REQUIRE(!item.SetValue("test"));
    REQUIRE(!item.GetValue().has_value());
    
    item.SetRowGrowable(true);
    REQUIRE( item.IsRowGrowable());
    
    wex::item item_int("int", wex::item::TEXTCTRL_INT, std::string("100"));
    REQUIRE( item_int.GetType() == wex::item::TEXTCTRL_INT);
    
    wex::item item_int2("int", wex::item::TEXTCTRL_INT, std::string("xxx"));
    REQUIRE( item_int2.GetType() == wex::item::TEXTCTRL_INT);
    item_int2.Layout(panel, sizer);
    REQUIRE( item_int2.GetWindow() != nullptr);
    try
    {
      // an excption should be raised as xxx cannot be converted to
      // a long.
      auto val = std::any_cast<long>(item_int2.GetValue());
      // therefore, we should not come here
      REQUIRE( 1 == 0);
    }
    catch (std::exception&)
    {
    }
    
    wex::item item_float("float", wex::item::TEXTCTRL_FLOAT, std::string("100.001"));
    REQUIRE( item_float.GetType() == wex::item::TEXTCTRL_FLOAT);
    
    wex::item item_spin("spindouble", 20.0, 30.0, 25.0, 0.1);
    REQUIRE( item_spin.GetType() == wex::item::SPINCTRLDOUBLE);

#ifdef __UNIX__
    wex::item item_picker("picker", wex::item::FILEPICKERCTRL, std::string("/usr/bin/git"));
#endif
    
    item.Layout(panel, sizer);
    REQUIRE( item.GetWindow() != nullptr);
    REQUIRE( std::any_cast<std::string>(item.GetValue()) == "hello string");
    REQUIRE( item.SetValue(std::string("value changed")));
    REQUIRE( std::any_cast<std::string>(item.GetValue()) == "value changed");
    REQUIRE( std::any_cast<std::string>(item.GetInitial()) == "hello string");
    
    REQUIRE( item_int.Layout(panel, sizer) != nullptr);
    REQUIRE( item_int.GetWindow() != nullptr);
    REQUIRE( std::any_cast<long>(item_int.GetValue()) == 100);
    REQUIRE( item_int.SetValue(300l));
    REQUIRE( std::any_cast<long>(item_int.GetValue()) == 300);

    // Write is tested in wex::item_dialog.
    
    item_float.Layout(panel, sizer);
    REQUIRE( std::any_cast<double>(item_float.GetValue()) == 100.001);

#ifdef __UNIX__
    REQUIRE( item_picker.Layout(panel, sizer) != nullptr);
    REQUIRE( std::any_cast<std::string>(item_picker.GetValue()) == "/usr/bin/git");
#endif
    
    std::vector <wex::item> items {item, item_int, item_spin
#ifdef __UNIX__
      , item_picker
#endif
      };
    
    const auto more(TestItems());
    items.insert(items.end(), more.begin(), more.end());
    
    // Layout the items and check control is created.
    for (auto& it : items)
    {
      // wex::item::USER is not yet laid out ok, gives errors.
      if (it.GetType() != wex::item::USER)
      {
        // Testing on not nullptr not possible,
        // not all items need a sizer.
        it.Layout(panel, sizer);
      }
   
      if (it.GetType() != wex::item::EMPTY && it.GetType() != wex::item::SPACER)
      {
        REQUIRE(it.GetWindow() != nullptr);
      }
    }
  }

  SUBCASE("Notebooks")
  {
    const std::vector<std::string> titles {
      "item::NOTEBOOK",
      "item::NOTEBOOK_AUI",
      "item::NOTEBOOK_CHOICE",
      "item::NOTEBOOK_EX",
      "item::NOTEBOOK_LIST",
      "item::NOTEBOOK_SIMPLE",
      "item::NOTEBOOK_TOOL",
      "item::NOTEBOOK_TREE"};
    
    REQUIRE(titles.size() == wex::item::NOTEBOOK_TREE - wex::item::NOTEBOOK + 1); 
    
    // Test dialog using notebook with pages.
    for (
      int style = wex::item::NOTEBOOK; 
      style <= wex::item::NOTEBOOK_TREE;
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
        {NotebookItem((wex::item::type)style, wex::item::LABEL_NONE, il)},
        wex::window_data().
          Button(wxOK | wxCANCEL | wxAPPLY).
          Title(titles[style - wex::item::NOTEBOOK]));
        
      dlg->Show();

      REQUIRE(std::any_cast<std::string>(dlg->GetItem("string1").GetInitial()) == "first");
      REQUIRE(std::any_cast<std::string>(dlg->GetItem("string1").GetValue()) == "first");
      REQUIRE(dlg->SetItemValue("string1", std::string("xxx")));
      REQUIRE(std::any_cast<std::string>(dlg->GetItem("string1").GetValue()) == "xxx");

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
