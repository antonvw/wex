////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>
#include "test.h"

void fixture::testFrameWithHistory()
{
  wxExListView* list = new wxExListView(
    m_Frame, wxExListView::LIST_HISTORY);

  AddPane(m_Frame, list);

  wxMenu* menu = new wxMenu();
  m_Frame->UseFileHistoryList(list);
  m_Frame->GetProjectHistory().UseMenu(1000, menu);
  list->Show();
  
  CPPUNIT_ASSERT(!m_Frame->OpenFile(GetTestFile())); // as we have no focused stc
  CPPUNIT_ASSERT(!m_Frame->GetFileHistory().GetHistoryFile().Contains("test.h"));

  CPPUNIT_ASSERT(!m_Frame->OpenFile(
    wxExFileName(m_Project),
    0,
    wxEmptyString,
    wxExFrameWithHistory::WIN_IS_PROJECT));

  std::vector<wxString> v{GetTestFile().GetFullPath()};
  CPPUNIT_ASSERT(m_Frame->FindInFiles(v, ID_TOOL_REPORT_FIND, false));

  // m_Frame->FindInFilesDialog(ID_TOOL_REPORT_FIND);
  CPPUNIT_ASSERT(!m_Frame->GetFindInCaption(ID_TOOL_REPORT_FIND).empty());
  
  // It does not open, next should fail.
  CPPUNIT_ASSERT(!m_Frame->GetProjectHistory().GetHistoryFile().Contains(m_Project));
  
  CPPUNIT_ASSERT( m_Frame->GetProject() == NULL);

  CPPUNIT_ASSERT(!m_Frame->Grep("xxxxxxx"));
  CPPUNIT_ASSERT(!m_Frame->Grep("xxxxxxx yyy"));
  CPPUNIT_ASSERT( m_Frame->Grep("xxxxxxx ./ *.cpp"));

  m_Frame->SetRecentProject("xxx.prj");
  CPPUNIT_ASSERT( m_Frame->GetProjectHistory().GetHistoryFile().empty());

  m_Frame->SetRecentFile(GetTestFile().GetFullPath());
}
