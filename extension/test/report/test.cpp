////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <TestCaller.h>
#include <wx/config.h>
#include <wx/extension/report/report.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExReportAppTestFixture::testConfig()
{
  wxConfig* cfg = new wxConfig(
    wxEmptyString, 
    wxEmptyString, 
    "test.cfg", 
    wxEmptyString, 
    wxCONFIG_USE_LOCAL_FILE);
    
  const int max = 100000;

  wxStopWatch sw;
  sw.Start();

  for (int j = 0; j < max; j++)
  {
    cfg->Read("test", 0l);
  }

  const long config = sw.Time();

  printf("reading %d items from config took %ld milliseconds\n", max, config);
}

void wxExReportAppTestFixture::testDirWithListView()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFile* listView = new wxExListViewFile(frame, frame, TEST_PRJ);
  
  wxExDirWithListView* dir = new wxExDirWithListView(listView, "./");
  CPPUNIT_ASSERT(dir->FindFiles());
}

void wxExReportAppTestFixture::testFrameWithHistory()
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
  
  CPPUNIT_ASSERT( frame->ProcessRun("wc test.h"));
  CPPUNIT_ASSERT( frame->ProcessIsSelected());
}

void wxExReportAppTestFixture::testListItem()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewStandard* listView = new wxExListViewStandard(
    frame, wxExListViewStandard::LIST_FILE);
  
  wxStopWatch sw;
  sw.Start();

  const int max = 250;
  for (int j = 0; j < max; j++)
  {
    wxExListItem item1(listView, wxExFileName("./test.h"));
    item1.Insert();
    wxExListItem item2(listView, wxExFileName("./test.cpp"));
    item2.Insert();
    wxExListItem item3(listView, wxExFileName("./main.cpp"));
    item3.Insert();
  }
  
  const long add = sw.Time();

  printf("adding %d items took %ld milliseconds\n", 3 * max, add);
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  printf("sorting %d items took %ld milliseconds\n", 3 * max, sort);
    
  CPPUNIT_ASSERT(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
}
  
void wxExReportAppTestFixture::testListViewFile()
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

void wxExReportAppTestFixture::testProcess()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  wxExProcess* process = new wxExProcess(frame, "wc test.h");
  CPPUNIT_ASSERT(process->IsSelected());
  CPPUNIT_ASSERT(process->Execute());
}

void wxExReportAppTestFixture::testSTCWithFrame()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExSTCWithFrame stc(frame, frame, wxExFileName(TEST_FILE));
  
  CPPUNIT_ASSERT(stc.GetFileName().GetFullPath().Contains("test.h"));
}
  
wxExReportTestSuite::wxExReportTestSuite()
  : CppUnit::TestSuite("wxexreport test suite")
{
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testConfig",
    &wxExReportAppTestFixture::testConfig));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testDirWithListView",
    &wxExReportAppTestFixture::testDirWithListView));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testFrameWithHistory",
    &wxExReportAppTestFixture::testFrameWithHistory));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testListItem",
    &wxExReportAppTestFixture::testListItem));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testListViewFile",
    &wxExReportAppTestFixture::testListViewFile));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testProcess",
    &wxExReportAppTestFixture::testProcess));
    
  addTest(new CppUnit::TestCaller<wxExReportAppTestFixture>(
    "testSTCWithFrame",
    &wxExReportAppTestFixture::testSTCWithFrame));
}
