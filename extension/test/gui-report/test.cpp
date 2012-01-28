////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/report.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExGuiReportTestFixture::testDirWithListView()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFile* listView = new wxExListViewFile(frame, frame, TEST_PRJ);
  
  wxExDirWithListView* dir = new wxExDirWithListView(listView, "./");
  CPPUNIT_ASSERT(dir->FindFiles());
}

void wxExGuiReportTestFixture::testFrameWithHistory()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  CPPUNIT_ASSERT(!frame->OpenFile(wxExFileName(TEST_FILE))); // as we have no focused stc
  CPPUNIT_ASSERT(!frame->GetRecentFile().Contains("test.h"));

  CPPUNIT_ASSERT(!frame->OpenFile(
    wxExFileName(TEST_PRJ),
    0,
    wxEmptyString,
    wxExFrameWithHistory::WIN_IS_PROJECT));

  // It does not open, next should fail.
  CPPUNIT_ASSERT(!frame->GetRecentProject().Contains("test-rep.prj"));
  
  CPPUNIT_ASSERT( frame->GetProcess()->Execute("wc test.h", wxEXEC_ASYNC));
  CPPUNIT_ASSERT( frame->GetProcess()->IsSelected());
}

void wxExGuiReportTestFixture::testListViewFile()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFile* listView = new wxExListViewFile(frame, frame, TEST_PRJ);
  
  listView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));

  // Remember that listview file already has columns.
  CPPUNIT_ASSERT(listView->FindColumn("String") > 1);
  CPPUNIT_ASSERT(listView->FindColumn("Number") > 1);

  CPPUNIT_ASSERT(listView->FileLoad(wxExFileName(TEST_PRJ)));
  CPPUNIT_ASSERT(listView->FileSave());

  CPPUNIT_ASSERT(listView->ItemFromText("test1\ntest2\n"));
}

void wxExGuiReportTestFixture::testListViewWithFrame()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  CPPUNIT_ASSERT(
    wxExListViewWithFrame::GetTypeTool(tool) == wxExListViewWithFrame::LIST_FIND);
}

void wxExGuiReportTestFixture::testProcess()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  wxExProcess* process = new wxExProcessListView(frame);
  CPPUNIT_ASSERT(process->Execute("wc test.h"));
  CPPUNIT_ASSERT(process->IsSelected());
}

void wxExGuiReportTestFixture::testSTCWithFrame()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExSTCWithFrame stc(frame, frame, wxExFileName(TEST_FILE));
  
  CPPUNIT_ASSERT(stc.GetFileName().GetFullPath().Contains("test.h"));
}
  
void wxExGuiReportTestFixture::testTextFileWithListView()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFileName* report = new wxExListViewFileName(
    frame, 
    wxExListViewFileName::LIST_FILE);
  
  CPPUNIT_ASSERT(wxExTextFileWithListView::SetupTool(tool, frame, report));
}

void wxExGuiReportTestFixture::test()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFileName* report = new wxExListViewFileName(
    frame, 
    wxExListViewFileName::LIST_FILE);
    
  wxArrayString files;
  
  CPPUNIT_ASSERT(wxDir::GetAllFiles(
    "../", 
    &files,
    "*.cpp", 
    wxDIR_FILES | wxDIR_DIRS) > 10);
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetFindString("xxxxxxxxxxxxx");
  
  CPPUNIT_ASSERT(frame->FindInSelection(
    files, 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  CPPUNIT_ASSERT(report->GetItemCount() == 1);
  
  frd->SetFindString("Author:");
  
  wxStopWatch sw;
  sw.Start();

  CPPUNIT_ASSERT(frame->FindInSelection(
    files, 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const long find = sw.Time();

  Report(wxString::Format(
    "match %d items in: %ld milliseconds", report->GetItemCount(), find));
  
  // Each file has one author, added the one above, and the one
  // in the header file. And there is already on item.
  CPPUNIT_ASSERT(report->GetItemCount() == (files.GetCount() + 3));
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxexreport test suite")
{
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testDirWithListView",
    &wxExGuiReportTestFixture::testDirWithListView));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testFrameWithHistory",
    &wxExGuiReportTestFixture::testFrameWithHistory));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testListViewFile",
    &wxExGuiReportTestFixture::testListViewFile));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testListViewWithFrame",
    &wxExGuiReportTestFixture::testListViewWithFrame));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testProcess",
    &wxExGuiReportTestFixture::testProcess));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testSTCWithFrame",
    &wxExGuiReportTestFixture::testSTCWithFrame));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testTextFileWithListView",
    &wxExGuiReportTestFixture::testTextFileWithListView));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "test",
    &wxExGuiReportTestFixture::test));
}
