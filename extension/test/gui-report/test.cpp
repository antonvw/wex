////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/report.h>
#include "test.h"

#define TEST_PRJ "./test-rep.prj"

wxExGuiReportTestFixture::wxExGuiReportTestFixture()
  : m_Project("test-rep.prj")
  , m_Frame((FrameWithHistory *)wxTheApp->GetTopWindow())
{
}

void wxExGuiReportTestFixture::test()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  
  wxExListViewFileName* report = new wxExListViewFileName(
    m_Frame, 
    wxExListViewFileName::LIST_FILE);
    
  wxArrayString files;
  
  CPPUNIT_ASSERT(wxDir::GetAllFiles(
    "../../../extension", 
    &files,
    "*.cpp", 
    wxDIR_FILES | wxDIR_DIRS) > 10);
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetUseRegEx(false);
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  CPPUNIT_ASSERT(m_Frame->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  CPPUNIT_ASSERT(report->GetItemCount() == 1);
  
  frd->SetFindString("Author:");
  
  wxStopWatch sw;
  sw.Start();

  CPPUNIT_ASSERT(m_Frame->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const long find = sw.Time();
  
  CPPUNIT_ASSERT(find < 1000);

  Report(wxString::Format(
    "wxExFrameWithHistory::FindInFiles %d items in: %ld ms", 
    report->GetItemCount(), find).ToStdString());
    
  wxLogMessage("%d %lu", 
    report->GetItemCount(), 
    wxExToVectorString(files).Get().size());
    
  // Each file has one author (files.GetCount()), add the one in SetFindString 
  // above, and the one that is already present on the 
  // list because of the first FindInFiles.
  CPPUNIT_ASSERT(report->GetItemCount() == (
    wxExToVectorString(files).Get().size() + 2));
}
