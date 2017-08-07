////////////////////////////////////////////////////////////////////////////////
// Name:      test-Item.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/numformatter.h>
#include <wx/extension/item.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include "../test-item.h"
#include "test.h"

TEST_CASE("wxExItem")
{
  wxPanel* panel = new wxPanel(GetFrame());
  AddPane(GetFrame(), panel);
  wxGridSizer* sizer = new wxGridSizer(3);
  panel->SetSizer(sizer);
  
  wxExItem::UseConfig(false);
  
  wxExItem item("item", "hello string", ITEM_TEXTCTRL, wxExControlData().Required(true));
  
  REQUIRE( item.GetColumns() == 1);
  REQUIRE( std::any_cast<wxString>(item.GetInitial()) == "hello string");
  REQUIRE( item.GetData().Required());
  REQUIRE( item.GetLabel() == "item");
  REQUIRE( item.GetPage().empty());
  REQUIRE( item.GetType() == ITEM_TEXTCTRL);
  REQUIRE( item.GetWindow() == nullptr);
  REQUIRE(!item.GetValue().has_value());
  REQUIRE(!item.IsRowGrowable());
  REQUIRE(!item.Apply());
  
  REQUIRE(!item.ToConfig(false));
  wxExItem::UseConfig(true);
  REQUIRE( item.ToConfig(false));
  wxExItem::UseConfig(false);
  
  item.SetDialog(nullptr);
  item.SetImageList(nullptr);
  
  // setting value if window is nullptr should have no effect.
  REQUIRE(!item.SetValue(wxString("test")));
  REQUIRE(!item.GetValue().has_value());
  
  item.SetRowGrowable(true);
  REQUIRE( item.IsRowGrowable());
  
  wxExItem item_int("int", ITEM_TEXTCTRL_INT, wxString("100"));
  REQUIRE( item_int.GetType() == ITEM_TEXTCTRL_INT);
  
  const char ds(wxNumberFormatter::GetDecimalSeparator());
  wxExItem item_float("float", ITEM_TEXTCTRL_FLOAT, wxString("100") + wxString(ds) + wxString("001"));
  REQUIRE( item_float.GetType() == ITEM_TEXTCTRL_FLOAT);
  
  wxExItem item_spin("spindouble", 20.0, 30.0, 25.0, 0.1);
  REQUIRE( item_spin.GetType() == ITEM_SPINCTRLDOUBLE);

#ifdef __UNIX__
  wxExItem item_picker("picker", ITEM_FILEPICKERCTRL, wxString("/usr/bin/git"));
#endif
  
#if wxCHECK_VERSION(3,1,0)
  item.Layout(panel, sizer);
  REQUIRE( item.GetWindow() != nullptr);
  REQUIRE( std::any_cast<wxString>(item.GetValue()) == "hello string");
  REQUIRE( item.SetValue(wxString("value changed")));
  REQUIRE( std::any_cast<wxString>(item.GetValue()) == "value changed");
  REQUIRE( std::any_cast<wxString>(item.GetInitial()) == "hello string");
  // TODO: Add Flags to window data.
  // REQUIRE( item.GetWindow()->GetWindowStyleFlag() == 1);
  
  item_int.Layout(panel, sizer);
  REQUIRE( item_int.GetWindow() != nullptr);
  REQUIRE( std::any_cast<long>(item_int.GetValue()) == 100);
  REQUIRE( item_int.SetValue(300l));
  REQUIRE( std::any_cast<long>(item_int.GetValue()) == 300);

  // Write is tested in wxExItemDialog.
  
  item_float.Layout(panel, sizer);
  REQUIRE( std::any_cast<double>(item_float.GetValue()) == 100.001);

#ifdef __UNIX__
  item_picker.Layout(panel, sizer);
  REQUIRE( std::any_cast<wxString>(item_picker.GetValue()) == "/usr/bin/git");
#endif
#endif
  
  std::vector <wxExItem> items {item, item_int, item_spin
#ifdef __UNIX__
    , item_picker
#endif
    };
  
  const auto more(TestItems());
  items.insert(items.end(), more.begin(), more.end());
  
#if wxCHECK_VERSION(3,1,0)
  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // ITEM_USER is not yet laid out ok, gives errors.
    if (it.GetType() != ITEM_USER)
    {
      // Testing on not nullptr not possible,
      // not all items need a sizer.
      it.Layout(panel, sizer);
    }
 
    if (it.GetType() != ITEM_EMPTY && it.GetType() != ITEM_SPACER)
    {
      REQUIRE(it.GetWindow() != nullptr);
    }
  }
#endif

  // Test notebooks.
  const std::vector<std::string> titles {
    "ITEM_NOTEBOOK",
    "ITEM_NOTEBOOK_AUI",
    "ITEM_NOTEBOOK_CHOICE",
    "ITEM_NOTEBOOK_EX",
    "ITEM_NOTEBOOK_LIST",
    "ITEM_NOTEBOOK_SIMPLE",
    "ITEM_NOTEBOOK_TOOL",
    "ITEM_NOTEBOOK_TREE"};
  
  REQUIRE(titles.size() == ITEM_NOTEBOOK_TREE - ITEM_NOTEBOOK + 1); 
  
  // Test dialog using notebook with pages.
  for (
    int style = ITEM_NOTEBOOK; 
    style <= ITEM_NOTEBOOK_TREE;
    style++)
  {
    wxImageList* il = nullptr;
    
    if (style == ITEM_NOTEBOOK_TOOL)
    {
      const wxSize imageSize(32, 32);

      il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
      
      il->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
    }

    wxExItemDialog* dlg = new wxExItemDialog(
      {NotebookItem((wxExItemType)style, LABEL_NONE, il)},
      wxExWindowData().
        Button(wxOK | wxCANCEL | wxAPPLY).
        Title(titles[style - ITEM_NOTEBOOK]));
      
    dlg->Show();

#if wxCHECK_VERSION(3,1,0)
    REQUIRE(std::any_cast<wxString>(dlg->GetItem("string1").GetInitial()) == "first");
    REQUIRE(std::any_cast<wxString>(dlg->GetItem("string1").GetValue()) == "first");
    REQUIRE(dlg->SetItemValue("string1", wxString("xxx")));
    REQUIRE(std::any_cast<wxString>(dlg->GetItem("string1").GetValue()) == "xxx");
#endif

    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
  }
  
  SUBCASE("wxExConfigDefaults")
  {
    wxExConfigDefaults def ({
      {"item1", ITEM_TEXTCTRL_INT, 1500l},
      {"item2", ITEM_TEXTCTRL_INT, 1510l}});
    REQUIRE(def.Get() != nullptr);
  }
}
