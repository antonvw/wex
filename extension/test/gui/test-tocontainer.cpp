////////////////////////////////////////////////////////////////////////////////
// Name:      test-to_container.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/generic/dirctrlg.h>
#include <wex/tostring.h>
#include <wex/managedframe.h>
#include <wex/util.h>
#include "test.h"

TEST_CASE("wex::to_container")
{
  wxComboBox* cb = new wxComboBox(frame(), wxID_ANY);
  wex::test::add_pane(frame(), cb);

  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
#ifdef __WXGTK__
  wxFileDialog dlg;
  wxGenericDirCtrl dir;
  REQUIRE( wex::to_vector_string(dlg).get().empty());
  REQUIRE( wex::to_vector_string(dir).get().empty());
#endif
  REQUIRE( wex::to_vector_string(a).get().size() == 4);
  REQUIRE( wex::to_vector_string("test test test").get().size() == 3);
  REQUIRE( wex::to_vector_string("test\\ test test").get().size() == 2);
  
#ifdef __WXGTK__
  REQUIRE( wex::to_list_string(dlg).get().empty());
  REQUIRE( wex::to_list_string(dir).get().empty());
#endif
  REQUIRE( wex::to_list_string(a).get().size() == 4);
  REQUIRE( wex::to_list_string("test test test").get().size() == 3);
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 5).get().size() == 0);
  
  wex::combobox_from_list(cb, std::list < std::string > {"x","y","z"});
  REQUIRE( wex::to_list_string(cb).get().size() == cb->GetCount());
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 2).get().size() == 2);
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 0).get().empty());
  
  cb->SetValue(wxEmptyString);
  REQUIRE( wex::to_list_string(cb).get().size() == cb->GetCount());

  cb->SetValue("other");
  REQUIRE( wex::to_list_string(cb).get().size() == cb->GetCount() + 1);
  REQUIRE( wex::to_list_string(cb).get().front() == "other");
}
