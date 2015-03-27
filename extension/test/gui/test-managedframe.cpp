////////////////////////////////////////////////////////////////////////////////
// Name:      test-managedm_Frame.cpp
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
#include <wx/extension/vi.h>
#include "test.h"

// Also test the toolbar (wxExToolBar).
void fixture::testManagedFrame()
{
  CPPUNIT_ASSERT(m_Frame->AllowClose(100, NULL));
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello world");
  wxExVi* vi = &stc->GetVi();
  
  CPPUNIT_ASSERT( m_Frame->ExecExCommand(ID_EDIT_NEXT) == NULL);
  
  m_Frame->GetExCommand(vi, "/");
  
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FOCUS_STC);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE);
  m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
  
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("VIBAR").IsShown());
  
  m_Frame->ShowExMessage("hello from m_Frame");
  
  m_Frame->SyncAll();
  m_Frame->SyncCloseAll(0);
  
  CPPUNIT_ASSERT( m_Frame->TogglePane("FINDBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("FINDBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->TogglePane("OPTIONSBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("OPTIONSBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->TogglePane("TOOLBAR"));
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("TOOLBAR").IsShown());
  CPPUNIT_ASSERT( m_Frame->TogglePane("VIBAR"));
  CPPUNIT_ASSERT( m_Frame->GetManager().GetPane("VIBAR").IsShown());
  
  CPPUNIT_ASSERT(!m_Frame->TogglePane("XXXXBAR"));
  CPPUNIT_ASSERT(!m_Frame->GetManager().GetPane("XXXXBAR").IsOk());
}
