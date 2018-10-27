////////////////////////////////////////////////////////////////////////////////
// Name:      test-config_item.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/vscroll.h>
#include <wex/config.h>
#include <wex/item.h>
#include <wex/managedframe.h>
#include "../test-configitem.h"
#include "test.h"

TEST_CASE("wex::config_item")
{
  wxScrolledWindow* panel = new wxScrolledWindow(GetFrame());
  AddPane(GetFrame(), panel);
  wxFlexGridSizer* sizer = new wxFlexGridSizer(4);
  panel->SetSizer(sizer);
  panel->SetScrollbars(20, 20, 50, 50);
  
  wex::item::UseConfig(true);

  // Use specific constructors.
  const wex::item ci_empty;
  const wex::item ci_spacer(5);
  const wex::item ci_cb("ci-cb", wex::item::COMBOBOX);
  const wex::item ci_cb_dir("ci-cb-dir", wex::item::COMBOBOX_DIR);
  const wex::item ci_sp("ci-sp", 1, 5);
  const wex::item ci_sp_d("ci-sp-d", 1.0, 5.0);
  const wex::item ci_sl("ci-sl", 1, 5, 2, wex::item::SLIDER);
  const wex::item ci_vl(wxLI_HORIZONTAL);
  wex::item ci_str("ci-string", std::string());
  const wex::item ci_hl("ci-hyper", "www.wxwidgets.org", wex::item::HYPERLINKCTRL);
  wex::item ci_st("ci-static", "HELLO", wex::item::STATICTEXT);
  const wex::item ci_int("ci-int",wex::item::TEXTCTRL_INT);
  const wex::item ci_rb("ci-rb", {
      {0, "Zero"},
      {1, "One"},
      {2, "Two"}},
    true);
  const wex::item ci_bc("ci-cl", {
      {0, "Bit One"},
      {1, "Bit Two"},
      {2, "Bit Three"},
      {4, "Bit Four"}},
    false);
  const wex::item ci_cl_n({"This","Or","Other"});
  const wex::item ci_user("ci-usr", 
    new wxTextCtrl(), 
    [=](wxWindow* user, wxWindow* parent, bool readonly) {
     ((wxTextCtrl*)user)->Create(parent, 100);}, 
    [=](wxWindow* user, bool save) {
      if (save) wex::config("mytext").set(((wxTextCtrl *)user)->GetValue().ToStdString());
      return true;},
    wex::item::LABEL_LEFT,
    [=](wxWindow* user, const std::any& value, bool save) {
      wxLogStatus(((wxTextCtrl *)user)->GetValue());});
  
  REQUIRE(ci_empty.GetType() == wex::item::EMPTY);
  REQUIRE(!ci_empty.IsRowGrowable());
  REQUIRE(ci_cb.GetType() == wex::item::COMBOBOX);
  REQUIRE(ci_cb_dir.GetType() == wex::item::COMBOBOX_DIR);
  REQUIRE(ci_spacer.GetType() == wex::item::SPACER);
  REQUIRE(ci_sl.GetLabel() == "ci-sl");
  REQUIRE(ci_sl.GetType() == wex::item::SLIDER);
  REQUIRE(ci_vl.GetType() == wex::item::STATICLINE);
  REQUIRE(ci_sp.GetLabel() == "ci-sp");
  REQUIRE(ci_sp.GetType() == wex::item::SPINCTRL);
  REQUIRE(ci_sp_d.GetType() == wex::item::SPINCTRLDOUBLE);
  REQUIRE(ci_str.GetType() == wex::item::TEXTCTRL);
  REQUIRE(ci_hl.GetType() == wex::item::HYPERLINKCTRL);
  REQUIRE(ci_st.GetType() == wex::item::STATICTEXT);
  REQUIRE(ci_int.GetType() == wex::item::TEXTCTRL_INT);
  REQUIRE(ci_rb.GetType() == wex::item::RADIOBOX);
  REQUIRE(ci_bc.GetType() == wex::item::CHECKLISTBOX_BIT);
  REQUIRE(ci_cl_n.GetType() == wex::item::CHECKLISTBOX_BOOL);
  REQUIRE(ci_user.GetType() == wex::item::USER);

  std::vector <wex::item> items {
    ci_empty, ci_spacer, ci_cb, ci_cb_dir, ci_sl, ci_vl, ci_sp, ci_sp_d,
    ci_str, ci_hl, ci_st, ci_int, ci_rb, ci_bc, ci_cl_n, ci_user};

  const auto more(TestConfigItems(0, 1));
  items.insert(items.end(), more.begin(), more.end());
  
  // Check members are initialized.
  for (auto& it : items)
  {
    REQUIRE( it.GetColumns() == 1);
    
    if (it.GetType() == wex::item::USER)
      REQUIRE( it.GetWindow() != nullptr);
    else 
      REQUIRE( it.GetWindow() == nullptr);
      
    if (
       it.GetType() != wex::item::STATICLINE &&
       it.GetType() != wex::item::SPACER &&
       it.GetType() != wex::item::EMPTY)
    {
      REQUIRE(!it.GetLabel().empty());
    }
    
    it.SetRowGrowable(true);
  }

  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // Testing on not nullptr not possible,
    // not all items need a sizer.
    it.Layout(panel, sizer);
 
    if (it.GetType() != wex::item::EMPTY && it.GetType() != wex::item::SPACER)
    {
      REQUIRE( it.GetWindow() != nullptr);
      
      if (
          it.GetType() == wex::item::CHECKLISTBOX_BOOL ||
         (it.GetType() >= wex::item::NOTEBOOK && it.GetType() <= wex::item::NOTEBOOK_TREE) || 
          it.GetType() == wex::item::RADIOBOX ||
          it.GetType() == wex::item::STATICLINE ||
          it.GetType() == wex::item::USER ||
          it.GetType() == wex::item::STATICTEXT)
      {
        REQUIRE(!it.GetValue().has_value());
      }
      else
      {
        REQUIRE( it.GetValue().has_value());
      }
    }
  }

  REQUIRE(ci_user.Apply());

  // Now check ToConfig (after Layout).  
  REQUIRE( ci_str.Layout(panel, sizer) != nullptr);
  REQUIRE( ci_st.Layout(panel, sizer) != nullptr);
  REQUIRE( ci_str.ToConfig(true));
  REQUIRE( ci_str.ToConfig(false));
  REQUIRE(!ci_st.ToConfig(true));
  REQUIRE(!ci_st.ToConfig(false));
  REQUIRE( ci_user.ToConfig(true));
  REQUIRE( ci_user.ToConfig(false));
}

TEST_CASE("wex::config_defaults")
{
  wex::config_defaults def({
    {"def-colour", wex::item::COLOURPICKERWIDGET, *wxWHITE},
    {"def-font", wex::item::FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)},
    {"def-double", wex::item::TEXTCTRL_FLOAT, 8.8},
    {"def-string", wex::item::TEXTCTRL, std::string("a string")},
    {"def-int", wex::item::TEXTCTRL_INT, 10l}});
  
  REQUIRE( wex::config("def-colour").exists());
}
