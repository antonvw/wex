////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/ctags.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExCTags")
{
  wxExSTCData data;

  SUBCASE("tags default")
  {
    wxExEx* ex = &GetSTC()->GetVi();

    REQUIRE( wxExCTags(ex).Find("wxExTestApp") );
    REQUIRE( wxExCTags(GetFrame()).Find("wxExTestApp") );
    REQUIRE( wxExCTags(ex).AutoComplete("wxExTest") == "wxExTestApp");

    wxExCTagsEntry current;
    wxExCTagsEntry filter;
    REQUIRE( wxExCTags(ex).Find("wxExTestApp", current, filter));
    REQUIRE( current.Kind() == "c" );
  }

  SUBCASE("tags non-existing file")
  {
    data.CTagsFileName("xxx");
    wxExSTC* stc = new wxExSTC(std::string("test"), data);
    AddPane(GetFrame(), stc);
    wxExEx* ex = &stc->GetVi();

    REQUIRE(!wxExCTags(ex).Find("wxExTestApp") );
  }
  
  SUBCASE("tags own file")
  {
    data.CTagsFileName("test-ctags");
    wxExSTC* stc = new wxExSTC(std::string("test"), data);
    AddPane(GetFrame(), stc);
    wxExEx* ex = &stc->GetVi();

    REQUIRE(!wxExCTags(ex).Find("") );
    REQUIRE(!wxExCTags(ex).Next() );
    REQUIRE(!wxExCTags(ex).Previous() );
    REQUIRE(!wxExCTags(ex).Find("xxxx") );
    REQUIRE( wxExCTags(ex).Find("wxExTestApp") );
    REQUIRE(!wxExCTags(ex).Next() );
    REQUIRE( wxExCTags(ex).Separator() != ' ');
  }
}
