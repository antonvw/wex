////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/printing.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::printing")
{
  REQUIRE(wex::printing::Get() != nullptr);
  REQUIRE(wex::printing::Get()->GetPrinter() != nullptr);
  REQUIRE(wex::printing::Get()->GetHtmlPrinter() != nullptr);
  wex::printing* old = wex::printing::Get();
  REQUIRE(wex::printing::Get()->Set(nullptr) == old);
  REQUIRE(wex::printing::Get(false) == nullptr);
  REQUIRE(wex::printing::Get(true) != nullptr);
  
  wex::printout* printout = new wex::printout(GetSTC());
  
  printout->OnPreparePrinting();
  int min, max, from, to;
  printout->GetPageInfo(&min, &max, &from, &to);
  REQUIRE(!printout->HasPage(5));
  REQUIRE(!printout->OnPrintPage(5));

  // these wait for a click
//  stc->Print(false);
//  stc->PrintPreview(wxPreviewFrame_NonModal);
}
