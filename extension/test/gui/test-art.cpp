////////////////////////////////////////////////////////////////////////////////
// Name:      test-art.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/art.h>
#include "test.h"

void fixture::testArt()
{
  CPPUNIT_ASSERT(!wxExStockArt(0).GetBitmap().IsOk());
  CPPUNIT_ASSERT(!wxExStockArt(wxID_ANY).GetBitmap().IsOk());
  CPPUNIT_ASSERT( wxExStockArt(wxID_NEW).GetBitmap().IsOk());
  CPPUNIT_ASSERT( wxExStockArt(wxID_OPEN).GetBitmap().IsOk());
}
