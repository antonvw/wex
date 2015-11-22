////////////////////////////////////////////////////////////////////////////////
// Name:      test-configitem.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/vscroll.h>
#include <wx/extension/configitem.h>
#include <wx/extension/managedframe.h>
#include "../test-configitem.h"
#include "test.h"

void fixture::testConfigItem()
{
  wxScrolledWindow* panel = new wxScrolledWindow(m_Frame);
  AddPane(m_Frame, panel);
  wxFlexGridSizer* sizer = new wxFlexGridSizer(4);
  panel->SetSizer(sizer);
  panel->SetScrollbars(20, 20, 50, 50);
  
  // Use specific constructors.
  const wxExConfigItem ci_empty;
  const wxExConfigItem ci_spacer(5);
  const wxExConfigItem ci_cb("ci-cb", ITEM_COMBOBOX);
  const wxExConfigItem ci_sp("ci-sp", 1, 5);
  const wxExConfigItem ci_sp_d("ci-sp-d", 1.0, 5.0);
  const wxExConfigItem ci_sl("ci-sl", 1, 5, wxEmptyString, ITEM_SLIDER);
  const wxExConfigItem ci_vl(wxLI_HORIZONTAL);
  wxExConfigItem ci_str("ci-string", wxEmptyString);
  const wxExConfigItem ci_hl("ci-hyper", "www.wxwidgets.org", wxEmptyString, 0,ITEM_HYPERLINKCTRL);
  wxExConfigItem ci_st("ci-static", "HELLO", wxEmptyString, 0, ITEM_STATICTEXT);
  const wxExConfigItem ci_int("ci-int",ITEM_INT);
  const wxExConfigItem ci_rb("ci-rb", 
    std::map<long, const wxString> {
      {0, "Zero"},
      {1, "One"},
      {2, "Two"}},
    true);
  const wxExConfigItem ci_bc("ci-cl", 
    std::map<long, const wxString> {
      {0, "Bit One"},
      {1, "Bit Two"},
      {2, "Bit Three"},
      {4, "Bit Four"}},
    false);
  const wxExConfigItem ci_cl_n(std::set<wxString> {"This","Or","Other"});
  const wxExConfigItem ci_user("ci-usr", new wxTextCtrl(), myTextCreate);
  
  CPPUNIT_ASSERT(ci_empty.GetType() == ITEM_EMPTY);
  CPPUNIT_ASSERT(!ci_empty.IsRowGrowable());
  CPPUNIT_ASSERT(ci_cb.GetType() == ITEM_COMBOBOX);
  CPPUNIT_ASSERT(ci_spacer.GetType() == ITEM_SPACER);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == ITEM_SLIDER);
  CPPUNIT_ASSERT(ci_vl.GetType() == ITEM_STATICLINE);
  CPPUNIT_ASSERT(ci_sp.GetLabel() == "ci-sp");
  CPPUNIT_ASSERT(ci_sp.GetType() == ITEM_SPINCTRL);
  CPPUNIT_ASSERT(ci_sp_d.GetType() == ITEM_SPINCTRL_DOUBLE);
  CPPUNIT_ASSERT(ci_str.GetType() == ITEM_STRING);
  CPPUNIT_ASSERT(ci_hl.GetType() == ITEM_HYPERLINKCTRL);
  CPPUNIT_ASSERT(ci_st.GetType() == ITEM_STATICTEXT);
  CPPUNIT_ASSERT(ci_int.GetType() == ITEM_INT);
  CPPUNIT_ASSERT(ci_rb.GetType() == ITEM_RADIOBOX);
  CPPUNIT_ASSERT(ci_bc.GetType() == ITEM_CHECKLISTBOX_BIT);
  CPPUNIT_ASSERT(ci_cl_n.GetType() == ITEM_CHECKLISTBOX_BOOL);
  CPPUNIT_ASSERT(ci_user.GetType() == ITEM_USER);

  std::vector <wxExConfigItem> items {
    ci_empty, ci_spacer, ci_cb, ci_sl, ci_vl, ci_sp, ci_sp_d,
    ci_str, ci_hl, ci_st, ci_int, ci_rb, ci_bc, ci_cl_n, ci_user};
  
  const auto more(TestConfigItems());
  items.insert(items.end(), more.begin(), more.end());
  
  // Check members are initialized.
  for (auto& it : items)
  {
    CPPUNIT_ASSERT( it.GetColumns() == -1);
    
    if (it.GetType() == ITEM_USER)
      CPPUNIT_ASSERT( it.GetWindow() != NULL);
    else 
      CPPUNIT_ASSERT( it.GetWindow() == NULL);
      
    if (
      it.GetType() != ITEM_STATICLINE &&
      it.GetType() != ITEM_SPACER &&
      it.GetType() != ITEM_EMPTY)
    {
      CPPUNIT_ASSERT(!it.GetLabel().empty());
    }
    
    it.SetRowGrowable(true);
    it.SetValidator(NULL);
  }

  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // ITEM_USER is not yet laid out ok, gives errors.
    if (it.GetType() != ITEM_USER)
    {
      // Testing on not NULL not possible,
      // not all items need a sizer.
      it.Layout(panel, sizer);
    }
 
    if (it.GetType() != ITEM_EMPTY && it.GetType() != ITEM_SPACER)
    {
      CPPUNIT_ASSERT(it.GetWindow() != NULL);
    }
  }

  // Now check ToConfig (after Layout).  
  CPPUNIT_ASSERT( ci_str.Layout(panel, sizer) != NULL);
  CPPUNIT_ASSERT( ci_st.Layout(panel, sizer) == NULL);
  CPPUNIT_ASSERT( ci_str.ToConfig(true));
  CPPUNIT_ASSERT( ci_str.ToConfig(false));
  CPPUNIT_ASSERT(!ci_st.ToConfig(true));
  CPPUNIT_ASSERT(!ci_st.ToConfig(false));
  
  // And check pages.
  const wxExConfigItem ci_str_page("ci-string-page", "test1", "page:3");
  CPPUNIT_ASSERT( ci_str_page.GetPage() == "page");
  CPPUNIT_ASSERT( ci_str_page.GetColumns() == 3);
  
  // Test wxExConfigDefaults
  wxExConfigDefaults def(std::vector<std::tuple<wxString, wxExItemType, wxAny>> {
    std::make_tuple("def-colour", ITEM_COLOUR, *wxWHITE),
    std::make_tuple("def-font", ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)),
    std::make_tuple("def-double", ITEM_FLOAT, 8.8),
    std::make_tuple("def-string", ITEM_STRING, "a string"),
    std::make_tuple("def-int", ITEM_INT, 10)});
  
  CPPUNIT_ASSERT( def.Get() != NULL);
  CPPUNIT_ASSERT( def.Get()->Exists("def-colour"));
}
