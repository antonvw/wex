////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/printing.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExPrinting")
{
  REQUIRE(wxExPrinting::Get() != nullptr);
  REQUIRE(wxExPrinting::Get()->GetPrinter() != nullptr);
  REQUIRE(wxExPrinting::Get()->GetHtmlPrinter() != nullptr);
  wxExPrinting* old = wxExPrinting::Get();
  REQUIRE(wxExPrinting::Get()->Set(nullptr) == old);
  REQUIRE(wxExPrinting::Get(false) == nullptr);
  REQUIRE(wxExPrinting::Get(true) != nullptr);
  
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello printing");
    
  wxExPrintout* printout = new wxExPrintout(stc);
  
  printout->OnPreparePrinting();
  int min, max, from, to;
  printout->GetPageInfo(&min, &max, &from, &to);
  REQUIRE(!printout->HasPage(5));
  REQUIRE(!printout->OnPrintPage(5));

  // these wait for a click
//  stc->Print(false);
//  stc->PrintPreview(wxPreviewFrame_NonModal);
}
