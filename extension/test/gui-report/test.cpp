////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/report.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_PRJ "./test-rep.prj"

void wxExGuiReportTestFixture::testDirCtrl()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  wxExGenericDirCtrl* ctrl = new wxExGenericDirCtrl(frame, frame);
  ctrl->ExpandAndSelectPath("test");

  wxCommandEvent event(ID_TREE_COPY);  
  wxPostEvent(ctrl, event);
}

void wxExGuiReportTestFixture::testDirTool()
{
  const wxExTool tool = ID_TOOL_REPORT_FIND;

  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  
  wxExListViewFileName* report = new wxExListViewFileName(
    frame, 
    wxExListViewFileName::LIST_FIND);
    
  if (!wxExTextFileWithListView::SetupTool(tool, frame, report))
  {
    return;
  }

  int flags = wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS;;
  
  wxExDirTool dir(
    tool,
    "./",
    "*.cpp",
    flags);

  dir.FindFiles();

  tool.Log(&dir.GetStatistics().GetElements());
}

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
  
  CPPUNIT_ASSERT( frame->SetRecentFile("xxx.cpp"));
  CPPUNIT_ASSERT(!frame->SetRecentProject("xxx.prj"));
  
  // frame->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  CPPUNIT_ASSERT(!frame->GetFindInCaption(ID_TOOL_REPORT_FIND).empty());
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

void wxExGuiReportTestFixture::testSTCWithFrame()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExSTCWithFrame stc(frame, frame, wxExFileName(TEST_FILE));
  
  CPPUNIT_ASSERT(stc.GetFileName().GetFullPath().Contains("test.h"));
  
  stc.PropertiesMessage();
}
  
void wxExGuiReportTestFixture::testTextFileWithListView()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  const wxExFileName fn(TEST_FILE);
  
  wxExListViewFileName* report = new wxExListViewFileName(
    frame, 
    wxExListViewFileName::LIST_FIND);
    
  wxExFindReplaceData::Get()->SetFindString("xx");
  
  CPPUNIT_ASSERT(wxExTextFileWithListView::SetupTool(tool, frame, report));
  
  wxExTextFileWithListView textFile(fn, tool);
  CPPUNIT_ASSERT( textFile.RunTool());
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT( textFile.RunTool()); // do the same test
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  wxExTextFileWithListView textFile2(fn, tool);
  CPPUNIT_ASSERT( textFile2.RunTool());
  CPPUNIT_ASSERT(!textFile2.GetStatistics().GetElements().GetItems().empty());
}

void wxExGuiReportTestFixture::testUtil()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExNotebook* notebook = new wxExNotebook(wxTheApp->GetTopWindow(), frame);
  
  wxWindow* page1 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page2 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  
  CPPUNIT_ASSERT( notebook->AddPage(page1, "page1") != NULL);
  CPPUNIT_ASSERT( notebook->AddPage(page2, "page2") != NULL);
  
  CPPUNIT_ASSERT( wxExForEach(notebook, ID_LIST_ALL_ITEMS));
  
  wxExListViewFileName* listView = new wxExListViewFileName(
    wxTheApp->GetTopWindow(), wxExListViewFileName::LIST_FILE);
  
  wxExListItem item(listView, wxExFileName("./test.h"));
  item.Insert();

  wxExFileStatistics stat = wxExRun(item, ID_TOOL_REPORT_KEYWORD);
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
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  CPPUNIT_ASSERT(frame->FindInFiles(
    files, 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  CPPUNIT_ASSERT(report->GetItemCount() == 1);
  
  frd->SetFindString("Author:");
  
  wxStopWatch sw;
  sw.Start();

  CPPUNIT_ASSERT(frame->FindInFiles(
    files, 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const long find = sw.Time();

  Report(wxString::Format(
    "wxExFrameWithHistory::FindInFiles %d items in: %ld ms", report->GetItemCount(), find).ToStdString());
  
  // Each file has one author (files.GetCount()), add the one in SetFindString above, 
  // and the one that is already present on the 
  // list because of the first FindInFiles.
  CPPUNIT_ASSERT(report->GetItemCount() == (files.GetCount() + 2));
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxExtension Report test suite")
{
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testDirCtrl",
    &wxExGuiReportTestFixture::testDirCtrl));

  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testDirTool",
    &wxExGuiReportTestFixture::testDirTool));

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
    "testSTCWithFrame",
    &wxExGuiReportTestFixture::testSTCWithFrame));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testTextFileWithListView",
    &wxExGuiReportTestFixture::testTextFileWithListView));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "testUtil",
    &wxExGuiReportTestFixture::testUtil));
    
  addTest(new CppUnit::TestCaller<wxExGuiReportTestFixture>(
    "test",
    &wxExGuiReportTestFixture::test));
}
