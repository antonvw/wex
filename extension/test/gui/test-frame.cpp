////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/frame.h>
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testFrame()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();
  frame->SetFocus(); // otherwise focus is on stc component caused by testEx

  std::vector<wxExStatusBarPane> panes;

  panes.push_back(wxExStatusBarPane());
  
  for (int i = 0; i < 25; i++)
  {
    panes.push_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
  }
  
  panes.push_back(wxExStatusBarPane("PaneInfo"));
  panes.push_back(wxExStatusBarPane("PaneLexer"));
  panes.push_back(wxExStatusBarPane("PaneFileType"));
  panes.push_back(wxExStatusBarPane("LastPane"));
  
  wxExStatusBar* sb = frame->SetupStatusBar(panes);
  CPPUNIT_ASSERT( sb != NULL);
  
  CPPUNIT_ASSERT( sb->GetFieldsCount () == panes.size());
  CPPUNIT_ASSERT( sb->SetStatusText("hello", ""));
  CPPUNIT_ASSERT( sb->SetStatusText("hello0", "Pane0"));
  CPPUNIT_ASSERT( sb->SetStatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( sb->SetStatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT(!sb->SetStatusText("hello3", "Panexxx"));
  CPPUNIT_ASSERT(!sb->SetStatusText("hello25", "Pane25"));
  CPPUNIT_ASSERT( sb->SetStatusText("GoodBye", "LastPane"));
  
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane1") == "hello1");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane2") == "hello2");
  CPPUNIT_ASSERT( sb->GetStatusText("Panexxx").empty());
  
  CPPUNIT_ASSERT( sb->ShowField("Pane0", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT(!sb->ShowField("Pane0", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0").empty());
  CPPUNIT_ASSERT( sb->ShowField("Pane0", true));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( sb->ShowField("LastPane", false));
  CPPUNIT_ASSERT( sb->GetStatusText("LastPane").empty());
  CPPUNIT_ASSERT(!sb->SetStatusText("BackAgain", "LastPane"));
  CPPUNIT_ASSERT( sb->ShowField("LastPane", true));
  CPPUNIT_ASSERT( sb->GetStatusText("LastPane") == "BackAgain");
  
  CPPUNIT_ASSERT(!frame->OpenFile(GetTestFile()));
  CPPUNIT_ASSERT( frame->OpenFile(GetTestFile().GetFullPath(), "contents"));
  
  CPPUNIT_ASSERT( frame->GetGrid() == NULL);
  CPPUNIT_ASSERT( frame->GetListView() == NULL);
  CPPUNIT_ASSERT( frame->GetSTC() == NULL);
  
  frame->SetFindFocus(NULL);
  frame->SetFindFocus(frame);
  frame->SetFindFocus(frame->GetSTC());
  
  wxMenuBar* bar = new wxMenuBar();
  frame->SetMenuBar(bar);
  
  frame->StatusBarClicked("test");
  frame->StatusBarClicked("Pane1");
  frame->StatusBarClicked("Pane2");
  
  frame->StatusBarClickedRight("test");
  frame->StatusBarClickedRight("Pane1");
  frame->StatusBarClickedRight("Pane2");
  
  CPPUNIT_ASSERT(!frame->StatusText("hello", "test"));
  CPPUNIT_ASSERT( frame->StatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( frame->StatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT( frame->GetStatusText("Pane1") = "hello1");
  CPPUNIT_ASSERT( frame->GetStatusText("Pane2") = "hello2");
  
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "test"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "Pane1"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "Pane2"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "PaneInfo"));
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  stc->SetFocus();
  
  CPPUNIT_ASSERT( frame->GetSTC() == stc);
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneInfo"));
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneLexer"));
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneFileType"));
}
