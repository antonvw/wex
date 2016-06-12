////////////////////////////////////////////////////////////////////////////////
// Name:      test-tocontainer.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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

TEST_CASE("wxExToContainer")
{
  wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
  AddPane(GetFrame(), cb);

  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
#ifdef __WXGTK__
  wxFileDialog dlg;
  wxGenericDirCtrl dir;
  REQUIRE( wxExToVectorString(dlg).Get().empty());
  REQUIRE( wxExToVectorString(dir).Get().empty());
#endif
  REQUIRE( wxExToVectorString(a).Get().size() == 4);
  REQUIRE( wxExToVectorString("test test test").Get().size() == 3);
  
#ifdef __WXGTK__
  REQUIRE( wxExToListString(dlg).Get().empty());
  REQUIRE( wxExToListString(dir).Get().empty());
#endif
  REQUIRE( wxExToListString(a).Get().size() == 4);
  REQUIRE( wxExToListString("test test test").Get().size() == 3);
  REQUIRE( wxExToContainer<std::list < wxString >>(cb, 5).Get().size() == 0);
  
  wxExComboBoxFromList(cb, std::list < wxString > {"x","y","z"});
  REQUIRE( wxExToListString(cb).Get().size() == 3);
  REQUIRE( wxExToContainer<std::list < wxString >>(cb, 2).Get().size() == 2);
  REQUIRE( wxExToContainer<std::list < wxString >>(cb, 0).Get().empty());
}
