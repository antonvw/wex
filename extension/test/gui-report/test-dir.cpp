////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>
#include "test.h"

void fixture::testDirTool()
{
  const wxExTool tool = ID_TOOL_REPORT_FIND;

  wxExListView* report = new wxExListView(
    m_Frame, 
    wxExListView::LIST_FIND);
    
  if (!wxExTextFileWithListView::SetupTool(tool, m_Frame, report))
  {
    return;
  }

  wxExDirTool dir(
    tool,
    "./",
    "*.cpp;*.h",
    wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS);

  dir.FindFiles();

  wxLogStatus(tool.Info(&dir.GetStatistics().GetElements()));
}

void fixture::testDirWithListView()
{
  wxExListViewFile* listView = new wxExListViewFile(m_Frame, m_Frame, m_Project);
  wxExDirWithListView* dir = new wxExDirWithListView(listView, GetTestDir());
  CPPUNIT_ASSERT(dir->FindFiles());
}
