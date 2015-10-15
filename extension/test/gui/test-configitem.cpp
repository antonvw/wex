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
#include <wx/extension/configitem.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testConfigItem()
{
  // Use specific constructors.
  const wxExConfigItem ci_empty;
  const wxExConfigItem ci_spacer(5);
  const wxExConfigItem ci_sl("ci-sl", 1, 5, wxEmptyString, CONFIG_SLIDER);
  const wxExConfigItem ci_vl(wxLI_HORIZONTAL);
  const wxExConfigItem ci_sp("ci-sp", 1, 5);
  const wxExConfigItem ci_sp_d("ci-sp-d", 1.0, 5.0, wxEmptyString, CONFIG_SPINCTRL_DOUBLE);
  const wxExConfigItem ci_sp_h("ci-sp-h", 1.0, 5.0, wxEmptyString, CONFIG_SPINCTRL_HEX);
  wxExConfigItem ci_str("ci-string", wxEmptyString);
  const wxExConfigItem ci_hl("ci-hyper", "www.wxwidgets.org", wxEmptyString, 0, CONFIG_HYPERLINKCTRL);
  wxExConfigItem ci_st("ci-static", "HELLO", wxEmptyString, 0, CONFIG_STATICTEXT);
  const wxExConfigItem ci_int("ci-int", CONFIG_INT);
  const wxExConfigItem ci_rb("ci-rb", 
    std::map<long, const wxString> {
      std::make_pair(0, "Zero"),
      std::make_pair(1, "One"),
      std::make_pair(2, "Two")},
    true);
  const wxExConfigItem ci_bc("ci-cl", 
    std::map<long, const wxString> {
      std::make_pair(0, "Bit One"),
      std::make_pair(1, "Bit Two"),
      std::make_pair(2, "Bit Three"),
      std::make_pair(4, "Bit Four")},
    false);
  const wxExConfigItem ci_cl_n(std::set<wxString> {"This","Or","Other"});
  const wxExConfigItem ci_user("ci-usr", new wxTextCtrl(), NULL);
  
  CPPUNIT_ASSERT(ci_empty.GetType() == CONFIG_EMPTY);
  CPPUNIT_ASSERT(!ci_empty.IsRowGrowable());
  CPPUNIT_ASSERT(ci_spacer.GetType() == CONFIG_SPACER);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == CONFIG_SLIDER);
  CPPUNIT_ASSERT(ci_vl.GetType() == CONFIG_STATICLINE);
  CPPUNIT_ASSERT(ci_sp.GetLabel() == "ci-sp");
  CPPUNIT_ASSERT(ci_sp.GetType() == CONFIG_SPINCTRL);
  CPPUNIT_ASSERT(ci_sp_d.GetType() == CONFIG_SPINCTRL_DOUBLE);
  CPPUNIT_ASSERT(ci_sp_h.GetType() == CONFIG_SPINCTRL_HEX);
  CPPUNIT_ASSERT(ci_str.GetType() == CONFIG_STRING);
  CPPUNIT_ASSERT(ci_hl.GetType() == CONFIG_HYPERLINKCTRL);
  CPPUNIT_ASSERT(ci_st.GetType() == CONFIG_STATICTEXT);
  CPPUNIT_ASSERT(ci_int.GetType() == CONFIG_INT);
  CPPUNIT_ASSERT(ci_rb.GetType() == CONFIG_RADIOBOX);
  CPPUNIT_ASSERT(ci_bc.GetType() == CONFIG_CHECKLISTBOX);
  CPPUNIT_ASSERT(ci_cl_n.GetType() == CONFIG_CHECKLISTBOX_NONAME);
  CPPUNIT_ASSERT(ci_user.GetType() == CONFIG_USER);

  std::vector <wxExConfigItem> items {
    ci_empty, ci_spacer, ci_sl, ci_vl, ci_sp, ci_sp_d, ci_sp_h,
    ci_str, ci_hl, ci_st, ci_int, ci_rb, ci_bc, ci_cl_n, ci_user};
  
  // Use general constructor, and add all items.
  for (
    int i = CONFIG_ITEM_MIN + 1;
    i < CONFIG_ITEM_MAX;
    i++)
  {
    if (i != CONFIG_USER)
    {
      items.push_back(wxExConfigItem(
        wxString::Format("item%d", i), 
        (wxExConfigType)i));
    }
  }

  // Check members are initialized.
  for (auto& it : items)
  {
    CPPUNIT_ASSERT( it.GetColumns() == -1);
    
    if (it.GetType() == CONFIG_USER)
      CPPUNIT_ASSERT( it.GetWindow() != NULL);
    else 
      CPPUNIT_ASSERT( it.GetWindow() == NULL);
      
    CPPUNIT_ASSERT(!it.GetIsRequired());
    
    if (
      it.GetType() != CONFIG_STATICLINE &&
      it.GetType() != CONFIG_SPACER &&
      it.GetType() != CONFIG_EMPTY)
    {
      CPPUNIT_ASSERT(!it.GetLabel().empty());
    }
    
    CPPUNIT_ASSERT( it.GetPage().empty());

    CPPUNIT_ASSERT(
      it.GetType() > CONFIG_ITEM_MIN &&
      it.GetType() < CONFIG_ITEM_MAX);
    
    it.SetRowGrowable(true);
    it.SetValidator(NULL);
  }

  wxGridSizer sizer(3);

  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // CONFIG_USER is not yet laid out ok, gives errors.
    if (it.GetType() != CONFIG_USER)
    {
      // Testing on not NULL not possible,
      // not all items need a sizer.
      it.Layout(m_Frame, &sizer);
    }
 
    if (it.GetType() != CONFIG_EMPTY && it.GetType() != CONFIG_SPACER)
    {
      CPPUNIT_ASSERT(it.GetWindow() != NULL);
    }
  }

  // Now check ToConfig (after Layout).  
  CPPUNIT_ASSERT( ci_str.Layout(m_Frame, &sizer) != NULL);
  CPPUNIT_ASSERT( ci_st.Layout(m_Frame, &sizer) == NULL);
  CPPUNIT_ASSERT( ci_str.ToConfig(true));
  CPPUNIT_ASSERT( ci_str.ToConfig(false));
  CPPUNIT_ASSERT(!ci_st.ToConfig(true));
  CPPUNIT_ASSERT(!ci_st.ToConfig(false));
  
  // And check pages.
  const wxExConfigItem ci_str_page("ci-string-page", "test1", "page:3");
  CPPUNIT_ASSERT( ci_str_page.GetPage() == "page");
  CPPUNIT_ASSERT( ci_str_page.GetColumns() == 3);
}
