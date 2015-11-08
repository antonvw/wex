////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/stc.h>
#include "test.h"

void fixture::testFrame()
{
  m_Frame->SetFocus(); // otherwise focus is on stc component caused by testEx

  CPPUNIT_ASSERT(!m_Frame->OpenFile(GetTestFile()));
  CPPUNIT_ASSERT( ((wxExFrame *)m_Frame)->OpenFile(GetTestFile().GetFullPath(), "contents"));
  
  CPPUNIT_ASSERT( m_Frame->GetGrid() == NULL);
  CPPUNIT_ASSERT( m_Frame->GetListView() == NULL);
  CPPUNIT_ASSERT( m_Frame->GetSTC() == NULL);
  
  m_Frame->SetFindFocus(NULL);
  m_Frame->SetFindFocus(m_Frame);
  m_Frame->SetFindFocus(m_Frame->GetSTC());
  
  wxMenuBar* bar = new wxMenuBar();
  m_Frame->SetMenuBar(bar);
  
  m_Frame->StatusBarClicked("test");
  m_Frame->StatusBarClicked("Pane1");
  m_Frame->StatusBarClicked("Pane2");
  
  m_Frame->StatusBarClickedRight("test");
  m_Frame->StatusBarClickedRight("Pane1");
  m_Frame->StatusBarClickedRight("Pane2");
  
  CPPUNIT_ASSERT(!m_Frame->StatusText("hello", "test"));
  CPPUNIT_ASSERT( m_Frame->StatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( m_Frame->StatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT( m_Frame->GetStatusText("Pane1") = "hello1");
  CPPUNIT_ASSERT( m_Frame->GetStatusText("Pane2") = "hello2");
  
  CPPUNIT_ASSERT(!m_Frame->UpdateStatusBar(m_Frame->GetSTC(), "test"));
  CPPUNIT_ASSERT(!m_Frame->UpdateStatusBar(m_Frame->GetSTC(), "Pane1"));
  CPPUNIT_ASSERT(!m_Frame->UpdateStatusBar(m_Frame->GetSTC(), "Pane2"));
  CPPUNIT_ASSERT(!m_Frame->UpdateStatusBar(m_Frame->GetSTC(), "PaneInfo"));
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  AddPane(m_Frame, stc);
  stc->SetFocus();
  
  CPPUNIT_ASSERT( m_Frame->GetSTC() == stc);
  CPPUNIT_ASSERT( m_Frame->UpdateStatusBar(stc, "PaneInfo"));
  CPPUNIT_ASSERT( m_Frame->UpdateStatusBar(stc, "PaneLexer"));
  CPPUNIT_ASSERT( m_Frame->UpdateStatusBar(stc, "PaneFileType"));
  
  for (auto id : std::vector<int> {
    wxID_FIND, wxID_REPLACE, 
    // wxID_OPEN,shows dialog..
    ID_VIEW_MENUBAR, ID_VIEW_STATUSBAR, ID_VIEW_TITLEBAR}) 
  {
    wxPostEvent(m_Frame, wxCommandEvent(wxEVT_MENU, id));
  }
}
