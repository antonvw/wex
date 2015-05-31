////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/address.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void fixture::testAddress()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  const int lines = stc->GetLineCount();
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLineAndSelect(1);
  ex->MarkerAdd('a'); // put marker a on line
  stc->GotoLineAndSelect(2);
  ex->MarkerAdd('b'); // put marker b on line
  
  CPPUNIT_ASSERT( wxExAddress(ex).GetLine() == 0);
  
  for (const auto& it : std::vector< std::pair<std::string, int>> {
    {"30", lines},
    {"40", lines},
    {"-40", 1},
    {"3-3", 0},
    {"3-1", 2},
    {".", 2},
    {".+1", 3},
    {"$", lines},
    {"$-2", lines - 2},
    {"x", 0},
    {"'x", 0},
    {"1,3s/x/y", 0},
    {"/2/", 3},
    {"?2?", 3},
    {"'a", 1},
    {"'b", 2},
    {"'b+10", lines},
    {"10+'b", lines},
    {"'a+'b", 3},
    {"'b+'a", 3},
    {"'b-'a", 1}})
  {
    CPPUNIT_ASSERT( wxExAddress(ex, it.first).GetLine() == it.second);
  }

  wxExAddress address3(ex, "5");
  
  // Test AdjustWindow.
  CPPUNIT_ASSERT( address3.AdjustWindow(""));
  CPPUNIT_ASSERT(!address3.AdjustWindow("xxx"));
  
  // Test Append.
  CPPUNIT_ASSERT( address3.Append("appended text"));
  CPPUNIT_ASSERT( stc->GetText().Contains("appended text"));
  
  // Test Flags.
  CPPUNIT_ASSERT( address3.Flags(""));
  CPPUNIT_ASSERT( address3.Flags("#"));
  CPPUNIT_ASSERT(!address3.Flags("x"));

  // Test GetLine.
  wxExAddress address(ex);
  CPPUNIT_ASSERT( address.GetLine() == 0);
  address.SetLine(-1);
  CPPUNIT_ASSERT( address.GetLine() == 1);
  address.SetLine(1);
  CPPUNIT_ASSERT( address.GetLine() == 1);
  address.SetLine(100);
  CPPUNIT_ASSERT( address.GetLine() == lines);
  
  wxExAddress address2(ex, "'a");
  CPPUNIT_ASSERT( address2.GetLine() == 1);
  address2.MarkerDelete();
  CPPUNIT_ASSERT( address2.GetLine() == 0);
  
  // Test Insert.
  CPPUNIT_ASSERT( address3.Insert("inserted text"));
  CPPUNIT_ASSERT( stc->GetText().Contains("inserted text"));
  
  // Test MarkerAdd.
  CPPUNIT_ASSERT( address3.MarkerAdd('x'));
  
  // Test MarkerDelete.
  CPPUNIT_ASSERT(!address3.MarkerDelete());
  CPPUNIT_ASSERT( wxExAddress(ex, "'x").MarkerDelete());
  
  // Test Put.
  ex->GetMacros().SetRegister('z', "zzzzz");
  CPPUNIT_ASSERT( address3.Put('z'));
  CPPUNIT_ASSERT( stc->GetText().Contains("zzzz"));
  
  // Test Read.
  CPPUNIT_ASSERT(!address3.Read("XXXXX"));
  CPPUNIT_ASSERT( address3.Read(GetTestDir() + "test.bin"));
  
  // Test Show.
  CPPUNIT_ASSERT( address3.Show());
}
