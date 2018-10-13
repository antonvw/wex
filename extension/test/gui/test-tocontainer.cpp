////////////////////////////////////////////////////////////////////////////////
// Name:      test-to_container.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/generic/dirctrlg.h>
#include <wx/extension/tostring.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/util.h>
#include "test.h"

TEST_CASE("wex::to_container")
{
  wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
#ifndef __WXOSX__
  // gives a warning about very large combobox
  AddPane(GetFrame(), cb);
#endif

  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
#ifdef __WXGTK__
  wxFileDialog dlg;
  wxGenericDirCtrl dir;
  REQUIRE( wex::to_vector_string(dlg).Get().empty());
  REQUIRE( wex::to_vector_string(dir).Get().empty());
#endif
  REQUIRE( wex::to_vector_string(a).Get().size() == 4);
  REQUIRE( wex::to_vector_string("test test test").Get().size() == 3);
  REQUIRE( wex::to_vector_string("test\\ test test").Get().size() == 2);
  
#ifdef __WXGTK__
  REQUIRE( wex::to_list_string(dlg).Get().empty());
  REQUIRE( wex::to_list_string(dir).Get().empty());
#endif
  REQUIRE( wex::to_list_string(a).Get().size() == 4);
  REQUIRE( wex::to_list_string("test test test").Get().size() == 3);
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 5).Get().size() == 0);
  
  wex::combobox_from_list(cb, std::list < std::string > {"x","y","z"});
  REQUIRE( wex::to_list_string(cb).Get().size() == cb->GetCount());
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 2).Get().size() == 2);
  REQUIRE( wex::to_container<std::list < std::string >>(cb, 0).Get().empty());
  
  cb->SetValue(wxEmptyString);
  REQUIRE( wex::to_list_string(cb).Get().size() == cb->GetCount());

  cb->SetValue("other");
  REQUIRE( wex::to_list_string(cb).Get().size() == cb->GetCount() + 1);
  REQUIRE( wex::to_list_string(cb).Get().front() == "other");
}
