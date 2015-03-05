////////////////////////////////////////////////////////////////////////////////
// Name:      test-textfile.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/buffer.h>
#include <wx/extension/textfile.h>
#include <wx/extension/frd.h>
#include "test.h"

void wxExGuiTestFixture::testFileStatistics()
{
  wxExFileStatistics fileStatistics;
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
  CPPUNIT_ASSERT(fileStatistics.Get("xx") == 0);

  wxExFileStatistics fileStatistics2;
  CPPUNIT_ASSERT(fileStatistics2.Get().empty());

  fileStatistics += fileStatistics2;
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
}

void wxExGuiTestFixture::testTextFile()
{
  // Test find.
  wxExTextFile textFile(GetTestFile(), ID_TOOL_REPORT_FIND);
  
  CPPUNIT_ASSERT( textFile.GetFileName() == GetTestFile());
  CPPUNIT_ASSERT( textFile.GetTool().GetId() == ID_TOOL_REPORT_FIND);
  
  wxExFindReplaceData::Get()->SetFindString("test");
  wxExFindReplaceData::Get()->SetMatchCase(true);
  wxExFindReplaceData::Get()->SetMatchWord(true);
  wxExFindReplaceData::Get()->SetUseRegEx(false);
  
  wxStopWatch sw;
  sw.Start();
  
  CPPUNIT_ASSERT( textFile.RunTool());
  
  const long elapsed = sw.Time();
  
  CPPUNIT_ASSERT(elapsed < 20);
  
  Report(wxString::Format(
    "wxExTextFile::matching %d items in %ld ms", 
    textFile.GetStatistics().Get(_("Actions Completed")), elapsed).ToStdString());
    
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT( textFile.GetStatistics().Get(_("Actions Completed")) == 193);
  
  // Test replace.
  wxExTextFile textFile2(GetTestFile(), ID_TOOL_REPORT_REPLACE);
  
  wxExFindReplaceData::Get()->SetReplaceString("test");
  
  wxStopWatch sw2;
  sw2.Start();
  CPPUNIT_ASSERT( textFile2.RunTool());
  const long elapsed2 = sw2.Time();
  
  CPPUNIT_ASSERT(elapsed2 < 100);
  
  Report(wxString::Format(
    "wxExTextFile::replacing %d items in %ld ms", 
    textFile2.GetStatistics().Get(_("Actions Completed")), elapsed2).ToStdString());
    
  CPPUNIT_ASSERT(!textFile2.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT( textFile2.GetStatistics().Get(_("Actions Completed")) == 194);
}
