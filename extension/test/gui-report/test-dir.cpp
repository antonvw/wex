////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>
#include "test.h"

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

  wxLogStatus(tool.Info(&dir.GetStatistics().GetElements()));
}

void wxExGuiReportTestFixture::testDirWithListView()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFile* listView = new wxExListViewFile(frame, frame, m_Project);
  
  wxExDirWithListView* dir = new wxExDirWithListView(listView, "./");
  CPPUNIT_ASSERT(dir->FindFiles());
}
