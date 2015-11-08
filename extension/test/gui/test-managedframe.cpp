////////////////////////////////////////////////////////////////////////////////
// Name:      test-managedframe.cpp
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
#include <wx/extension/toolbar.h>
#include <wx/extension/vi.h>
#include "test.h"

// Also test the toolbar (wxExToolBar).
void fixture::testManagedFrame()
{
  CPPUNIT_ASSERT(m_Frame->AllowClose(100, NULL));
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello world");
  AddPane(m_Frame, stc);
  
  stc->SetFocus();
  stc->Show();
  wxExVi* vi = &stc->GetVi();
  
  wxExSTC* stc2 = NULL;  
  CPPUNIT_ASSERT(!m_Frame->ExecExCommand(":n", stc2));
  CPPUNIT_ASSERT( stc2 == NULL);
  
  m_Frame->GetExCommand(vi, "/");
  
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FOCUS_STC);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
  
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("VIBAR").IsShown());
  
  m_Frame->GetFileHistory().Clear();
  
  wxMenu* menu = new wxMenu();
  m_Frame->GetFileHistory().UseMenu(1000, menu);
  m_Frame->SetFindFocus(m_Frame->GetSTC());
  CPPUNIT_ASSERT( m_Frame->OpenFile(GetTestFile()));
  
  m_Frame->SetRecentFile(GetTestFile().GetFullPath());
  m_Frame->SetRecentFile("testing");
  
  CPPUNIT_ASSERT( m_Frame->GetFileHistory().GetHistoryFile().Contains("test.h"));
  CPPUNIT_ASSERT( m_Frame->GetFileHistory().GetCount() > 0);
  CPPUNIT_ASSERT(!m_Frame->GetFileHistory().GetVector(5).empty());
  
  m_Frame->ShowExMessage("hello from m_Frame");
  CPPUNIT_ASSERT(!m_Frame->ShowPane("xxxx"));
  CPPUNIT_ASSERT(!m_Frame->ShowPane("xxxx", false));
  m_Frame->PrintEx(vi, "hello vi");
  
  m_Frame->SyncAll();
  m_Frame->SyncCloseAll(0);
  
  CPPUNIT_ASSERT( m_Frame->GetToolBar() != NULL);
  CPPUNIT_ASSERT( m_Frame->GetOptionsToolBar() != NULL);
  
  m_Frame->GetToolBar()->AddControls();
  CPPUNIT_ASSERT( m_Frame->TogglePane("FINDBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("FINDBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->TogglePane("OPTIONSBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("OPTIONSBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->TogglePane("TOOLBAR"));
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("TOOLBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->ShowPane("TOOLBAR"));
  CPPUNIT_ASSERT( m_Frame->TogglePane("VIBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("VIBAR").IsShown());
  
  CPPUNIT_ASSERT(!m_Frame->TogglePane("XXXXBAR"));
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("XXXXBAR").IsOk());
  
  m_Frame->OnNotebook(100, stc);
  
  m_Frame->AppendPanes(menu);

  for (auto id : std::vector<int> {
    wxID_PREFERENCES, ID_FIND_FIRST, 
    ID_VIEW_LOWEST + 1, ID_VIEW_LOWEST + 2}) 
  {
    wxPostEvent(m_Frame, wxCommandEvent(wxEVT_MENU, id));
  }
}
