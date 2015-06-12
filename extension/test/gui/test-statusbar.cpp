////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/statusbar.h>
#include "test.h"

void fixture::testStatusBar()
{
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello", ""));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello0", "Pane0"));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello3", "Pane3"));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("hello4", "Pane4"));
  CPPUNIT_ASSERT(!m_StatusBar->SetStatusText("helloxxx", "Panexxx"));
  CPPUNIT_ASSERT(!m_StatusBar->SetStatusText("hello25", "Pane25"));
  CPPUNIT_ASSERT( m_StatusBar->SetStatusText("GoodBye", "LastPane"));

  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( ((wxStatusBar*) m_StatusBar)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Pane1") == "hello1");
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Pane2") == "hello2");
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Panexxx").empty());
  
  CPPUNIT_ASSERT( m_StatusBar->ShowField("Pane0", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) m_StatusBar)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT(!m_StatusBar->ShowField("Pane0", false));
  CPPUNIT_ASSERT( m_StatusBar->ShowField("Pane3", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) m_StatusBar)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Pane0").empty());
  CPPUNIT_ASSERT( m_StatusBar->ShowField("Pane0", true));
  CPPUNIT_ASSERT( ((wxStatusBar*) m_StatusBar)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( m_StatusBar->ShowField("LastPane", false));
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("LastPane").empty());
  CPPUNIT_ASSERT(!m_StatusBar->SetStatusText("BackAgain", "LastPane"));
  CPPUNIT_ASSERT( m_StatusBar->ShowField("LastPane", true));
  CPPUNIT_ASSERT( m_StatusBar->GetStatusText("LastPane") == "BackAgain");
}
