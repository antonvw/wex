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

  wxExListViewFileName* report = new wxExListViewFileName(
    m_Frame, 
    wxExListViewFileName::LIST_FIND);
    
  if (!wxExTextFileWithListView::SetupTool(tool, m_Frame, report))
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
  wxExListViewFile* listView = new wxExListViewFile(m_Frame, m_Frame, m_Project);
  wxExDirWithListView* dir = new wxExDirWithListView(listView, GetTestDir());
  CPPUNIT_ASSERT(dir->FindFiles());
}
