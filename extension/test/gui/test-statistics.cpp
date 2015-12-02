////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/statistics.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testStatistics()
{
  wxExStatistics<int>* statistics1 = new wxExStatistics<int>;
  wxExStatistics<int>* statistics2 = new wxExStatistics<int>;
  
  CPPUNIT_ASSERT(statistics1->Get().empty());
  CPPUNIT_ASSERT(statistics1->Get("xx") == 0);
  
  CPPUNIT_ASSERT(statistics2->Inc("xx") == 1);
  CPPUNIT_ASSERT(statistics2->Set("xx", 3) == 3);
  CPPUNIT_ASSERT(statistics2->Dec("xx") == 2);

  *statistics1 += *statistics2;
  
  CPPUNIT_ASSERT(!statistics1->Get().empty());
  CPPUNIT_ASSERT( statistics1->Get("xx") == 2);

  wxExGrid* grid1 = statistics1->Show(m_Frame);
  CPPUNIT_ASSERT(grid1 != nullptr);
  AddPane(m_Frame, grid1);
  
  wxExGrid* grid2 = statistics2->Show(m_Frame);
  AddPane(m_Frame, grid2);
  
  CPPUNIT_ASSERT(statistics1->Show(m_Frame) == grid1);
  CPPUNIT_ASSERT(statistics2->Set("xx", 10) == 10);
  
  statistics2->Clear();
  CPPUNIT_ASSERT(statistics2->Inc("xx") == 1);
  
  wxPostEvent(grid2, wxCommandEvent(wxEVT_MENU, wxID_CLEAR));
}
