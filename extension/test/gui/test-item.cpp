////////////////////////////////////////////////////////////////////////////////
// Name:      test-Item.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h>
#include <wx/imaglist.h>
#include <wx/extension/item.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/managedframe.h>
#include "../test-item.h"
#include "test.h"

void fixture::testItem()
{
  wxPanel* panel = new wxPanel(m_Frame);
  AddPane(m_Frame, panel);
  wxGridSizer* sizer = new wxGridSizer(3);
  panel->SetSizer(sizer);
  
  wxExItem::UseConfig(false);
  
  wxExItem item("item", "hello string", 1, ITEM_TEXTCTRL, true);
  
  CPPUNIT_ASSERT( item.GetColumns() == 1);
  CPPUNIT_ASSERT( item.GetInitial().As<wxString>() == "hello string");
  CPPUNIT_ASSERT( item.GetIsRequired());
  CPPUNIT_ASSERT( item.GetLabel() == "item");
  CPPUNIT_ASSERT( item.GetPage().empty());
  CPPUNIT_ASSERT( item.GetType() == ITEM_TEXTCTRL);
  CPPUNIT_ASSERT( item.GetWindow() == nullptr);
  CPPUNIT_ASSERT( item.GetValue().IsNull());
  CPPUNIT_ASSERT(!item.IsRowGrowable());
  CPPUNIT_ASSERT(!item.IsNotebook());
  
  CPPUNIT_ASSERT(!item.ToConfig(false));
  wxExItem::UseConfig(true);
  CPPUNIT_ASSERT( item.ToConfig(false));
  wxExItem::UseConfig(false);
  
  item.SetDialog(nullptr);
  item.SetValidator(nullptr);
  
  // setting value if window is nullptr should have no effect.
  CPPUNIT_ASSERT(!item.SetValue(wxString("test")));
  CPPUNIT_ASSERT( item.GetValue().IsNull());
  
  item.SetRowGrowable(true);
  CPPUNIT_ASSERT( item.IsRowGrowable());
  
  wxExItem item_int("int", ITEM_TEXTCTRL_INT, "100");
  CPPUNIT_ASSERT( item_int.GetType() == ITEM_TEXTCTRL_INT);
  
  wxExItem item_spin("spindouble", 20.0, 30.0, 25.0, 0.1);
  CPPUNIT_ASSERT( item_spin.GetType() == ITEM_SPINCTRLDOUBLE);
  
  wxExItem item_picker("picker", ITEM_FILEPICKERCTRL, "/usr/bin/git");
  
#if wxCHECK_VERSION(3,1,0)
  item.Layout(panel, sizer);
  CPPUNIT_ASSERT( item.GetWindow() != nullptr);
  CPPUNIT_ASSERT( item.GetValue() == "hello string");
  CPPUNIT_ASSERT( item.SetValue(wxString("value changed")));
  CPPUNIT_ASSERT( item.GetValue() == "value changed");
  CPPUNIT_ASSERT( item.GetInitial().As<wxString>() == "hello string");
  CPPUNIT_ASSERT( item.GetWindow()->GetWindowStyleFlag() == 1);
  
  item_int.Layout(panel, sizer);
  CPPUNIT_ASSERT( item_int.GetWindow() != nullptr);
  CPPUNIT_ASSERT( item_int.GetValue() == 100);
  CPPUNIT_ASSERT( item_int.SetValue(300));
  CPPUNIT_ASSERT( item_int.GetValue() == 300);
  
  item_picker.Layout(panel, sizer);
  CPPUNIT_ASSERT( item_picker.GetValue() == "/usr/bin/git");
#endif
  
  std::vector <wxExItem> items {
    item, item_int, item_spin, item_picker};
  
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
      CPPUNIT_ASSERT(it.GetWindow() != nullptr);
    }
  }
#endif

  // Test notebooks.
  const std::vector<wxString> titles {
    "ITEM_NOTEBOOK",
    "ITEM_NOTEBOOK_AUI",
    "ITEM_NOTEBOOK_CHOICE",
    "ITEM_NOTEBOOK_EX",
    "ITEM_NOTEBOOK_LIST",
    "ITEM_NOTEBOOK_SIMPLE",
    "ITEM_NOTEBOOK_TOOL",
    "ITEM_NOTEBOOK_TREE"};
  
  CPPUNIT_ASSERT(titles.size() == ITEM_NOTEBOOK_TREE - ITEM_NOTEBOOK + 1); 
  
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
    
    const wxExItem notebook("notebook", wxExItem::ItemsNotebook {
      {wxString("page0"), 
        {wxExItem("subnotebook", wxExItem::ItemsNotebook {
          {"strings", 
            {wxExItem("string1", "first"),
             wxExItem("string2"),
             wxExItem("string3")}},
          {"checkboxes", 
           {wxExItem("checkbox1", ITEM_CHECKBOX),
            wxExItem("checkbox2", ITEM_CHECKBOX),
            wxExItem("checkbox3", ITEM_CHECKBOX),
            wxExItem("checkbox4", ITEM_CHECKBOX)}},
          {"spins", 
            {wxExItem("spin1", 0, 10),
             wxExItem("spin2", 0, 10),
             wxExItem("spin3", 0, 10),
             wxExItem("spin control double", 10.1, 15.0, 11.0, 0.1)}}}, ITEM_NOTEBOOK),
         wxExItem("string1", "nice"),
         wxExItem("string2"),
         wxExItem("string3")}},
      {"page1", 
        {wxExItem("string1", "nice"),
         wxExItem("string2"),
         wxExItem("string3")}},
      {"checkboxes", 
       {wxExItem("checkbox1", ITEM_CHECKBOX),
        wxExItem("checkbox2", ITEM_CHECKBOX),
        wxExItem("checkbox3", ITEM_CHECKBOX),
        wxExItem("checkbox4", ITEM_CHECKBOX)}},
      {"spins", 
        {wxExItem("spin1", 0, 10),
         wxExItem("spin2", 0, 10),
         wxExItem("spin3", 0, 10),
         wxExItem("spin control double", 10.1, 15.0, 11.0, 0.1)}}}, 
      (wxExItemType)style, 0, 0, 1, LABEL_NONE, il);
    
    wxExItemDialog* dlg = new wxExItemDialog(
      m_Frame, 
      std::vector <wxExItem> {notebook},
      titles[style - ITEM_NOTEBOOK],
      0,
      1,
      wxOK | wxCANCEL | wxAPPLY);
      
    dlg->Show();
    
#if wxCHECK_VERSION(3,1,0)
    CPPUNIT_ASSERT(dlg->GetItem("string1").GetValue() == "first");
    CPPUNIT_ASSERT(dlg->SetItemValue("string1", "xxx"));
    CPPUNIT_ASSERT(dlg->GetItem("string1").GetValue() == "xxx");
#endif

    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
  }
}
