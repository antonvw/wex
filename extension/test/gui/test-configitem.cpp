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

void wxExGuiTestFixture::testConfigItem()
{
  std::vector <wxExConfigItem> items;

  // Use specific constructors.
  wxExConfigItem ci_empty(5);
  items.push_back(ci_empty);
  CPPUNIT_ASSERT(ci_empty.GetType() == CONFIG_EMPTY);
    
  wxExConfigItem ci_sl("ci-sl", 1, 5, 
    wxEmptyString, CONFIG_SLIDER);
  items.push_back(ci_sl);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == CONFIG_SLIDER);

  wxExConfigItem ci_vl(wxLI_HORIZONTAL, wxEmptyString);
  items.push_back(ci_vl);
  CPPUNIT_ASSERT(ci_vl.GetType() == CONFIG_STATICLINE);
    
  wxExConfigItem ci_sp("ci-sp", 1, 5);
  items.push_back(ci_sp);
  CPPUNIT_ASSERT(ci_sp.GetLabel() == "ci-sp");
  CPPUNIT_ASSERT(ci_sp.GetType() == CONFIG_SPINCTRL);
  
  wxExConfigItem ci_sp_d("ci-sp-d", 1.0, 5.0,
    wxEmptyString, CONFIG_SPINCTRL_DOUBLE);
  items.push_back(ci_sp_d);
  CPPUNIT_ASSERT(ci_sp_d.GetType() == CONFIG_SPINCTRL_DOUBLE);
  
  wxExConfigItem ci_sp_h("ci-sp-h", 1.0, 5.0,
    wxEmptyString, CONFIG_SPINCTRL_HEX);
  items.push_back(ci_sp_h);
  CPPUNIT_ASSERT(ci_sp_h.GetType() == CONFIG_SPINCTRL_HEX);
  
  wxExConfigItem ci_str("ci-string");
  items.push_back(ci_str);
  CPPUNIT_ASSERT(ci_str.GetType() == CONFIG_STRING);
  
  wxExConfigItem ci_hl("ci-hyper",
    "www.wxwidgets.org", wxEmptyString, 0, CONFIG_HYPERLINKCTRL);
  items.push_back(ci_hl);
  CPPUNIT_ASSERT(ci_hl.GetType() == CONFIG_HYPERLINKCTRL);

  wxExConfigItem ci_st("ci-static",
    "HELLO", wxEmptyString, 0, CONFIG_STATICTEXT);
  items.push_back(ci_st);
  CPPUNIT_ASSERT( ci_st.GetType() == CONFIG_STATICTEXT);
  
  wxExConfigItem ci_int("ci-int", CONFIG_INT);
  items.push_back(ci_int);
  CPPUNIT_ASSERT(ci_int.GetType() == CONFIG_INT);

  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(0, "Zero"));
  echoices.insert(std::make_pair(1, "One"));
  echoices.insert(std::make_pair(2, "Two"));
  wxExConfigItem ci_rb("ci-rb", echoices, true);
  items.push_back(ci_rb);
  CPPUNIT_ASSERT(ci_rb.GetType() == CONFIG_RADIOBOX);

  std::map<long, const wxString> cl;
  cl.insert(std::make_pair(0, "Bit One"));
  cl.insert(std::make_pair(1, "Bit Two"));
  cl.insert(std::make_pair(2, "Bit Three"));
  cl.insert(std::make_pair(4, "Bit Four"));
  wxExConfigItem ci_bc("ci-cl", cl, false);
  items.push_back(ci_bc);
  CPPUNIT_ASSERT(ci_bc.GetType() == CONFIG_CHECKLISTBOX);

  std::set<wxString> bchoices;
  bchoices.insert("This");
  bchoices.insert("Or");
  bchoices.insert("Other");
  wxExConfigItem ci_cl_n(bchoices);
  items.push_back(ci_cl_n);
  CPPUNIT_ASSERT(ci_cl_n.GetType() == CONFIG_CHECKLISTBOX_NONAME);
  
  wxExConfigItem ci_user("ci-usr", new wxTextCtrl(), NULL);
  items.push_back(ci_user);
  CPPUNIT_ASSERT(ci_user.GetType() == CONFIG_USER);

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
      it.GetType() != CONFIG_EMPTY)
    {
      CPPUNIT_ASSERT(!it.GetLabel().empty());
    }
    
    CPPUNIT_ASSERT( it.GetPage().empty());

    CPPUNIT_ASSERT(
      it.GetType() > CONFIG_ITEM_MIN &&
      it.GetType() < CONFIG_ITEM_MAX);
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
 
    if (it.GetType() != CONFIG_EMPTY)
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
}
