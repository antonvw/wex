////////////////////////////////////////////////////////////////////////////////
// Name:      test-printing.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testPrinting()
{
  CPPUNIT_ASSERT(wxExPrinting::Get() != nullptr);
  CPPUNIT_ASSERT(wxExPrinting::Get()->GetPrinter() != nullptr);
  CPPUNIT_ASSERT(wxExPrinting::Get()->GetHtmlPrinter() != nullptr);
  wxExPrinting* old = wxExPrinting::Get();
  CPPUNIT_ASSERT(wxExPrinting::Get()->Set(nullptr) == old);
  CPPUNIT_ASSERT(wxExPrinting::Get(false) == nullptr);
  CPPUNIT_ASSERT(wxExPrinting::Get(true) != nullptr);
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello printing");
    
  new wxExPrintout(stc);
  
//  stc->Print(false);
//  stc->PrintPreview(wxPreviewFrame_NonModal);
}
