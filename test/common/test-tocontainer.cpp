////////////////////////////////////////////////////////////////////////////////
// Name:      test-tocontainer.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/tostring.h>
#include <wex/common/util.h>
#include <wx/generic/dirctrlg.h>

#include "test.h"

TEST_CASE("wex::to_container")
{
  auto* cb = new wxComboBox(get_frame(), wxID_ANY);

  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");

  wxFileDialog     dlg(get_frame());
  wxGenericDirCtrl dir(get_frame());

  REQUIRE(wex::to_vector_string(dlg).get().empty());
#ifndef __WXMSW__
  REQUIRE(!wex::to_vector_string(dir).get().empty());
#endif
  REQUIRE(wex::to_vector_string(a).get().size() == 4);
  REQUIRE(wex::to_vector_path(a).get().size() == 4);
  REQUIRE(wex::to_vector_string("test test test").get().size() == 3);
  REQUIRE(wex::to_vector_string("test\\ test\\ test").get().size() == 1);
  REQUIRE(
    wex::to_vector_string("test\\ test\\ test").get().front() ==
    "test test test");

  REQUIRE(wex::to_vector_string("test\\ ").get().size() == 1);

  REQUIRE(wex::to_list_string(dlg).get().empty());
#ifndef __WXMSW__
  REQUIRE(!wex::to_list_string(dir).get().empty());
#endif
  REQUIRE(wex::to_list_string(a).get().size() == 4);
  REQUIRE(wex::to_list_string("test test test").get().size() == 3);
  REQUIRE(wex::to_container<wex::strings_t>(cb, 5).get().size() == 1);

  wex::combobox_from_list(cb, wex::strings_t{"x", "y", "z"});
  REQUIRE(wex::to_list_string(cb).get().size() == cb->GetCount());
  REQUIRE(wex::to_container<wex::strings_t>(cb, 2).get().size() == 2);
  REQUIRE(wex::to_container<wex::strings_t>(cb, 0).get().empty());

  cb->SetValue(wxEmptyString);
  REQUIRE(wex::to_list_string(cb).get().size() == cb->GetCount() + 1);

  cb->SetValue("other");
  REQUIRE(wex::to_list_string(cb).get().size() == cb->GetCount() + 1);
  REQUIRE(wex::to_list_string(cb).get().front() == "other");
}
