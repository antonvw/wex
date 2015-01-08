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
#include <wx/extension/vi.h>
#include "test.h"

// Also test the toolbar (wxExToolBar).
void wxExGuiTestFixture::testManagedFrame()
{
  wxExManagedFrame* frame = (wxExManagedFrame*)wxTheApp->GetTopWindow();

  CPPUNIT_ASSERT(frame->AllowClose(100, NULL));
  
  wxExSTC* stc = new wxExSTC(frame, "hello world");
  wxExVi* vi = &stc->GetVi();
  
  CPPUNIT_ASSERT( frame->ExecExCommand(ID_EDIT_NEXT) == NULL);
  
  frame->GetExCommand(vi, "/");
  
  frame->HideExBar();
  frame->HideExBar(false);
  
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("VIBAR").IsShown());
  
  frame->ShowExMessage("hello from frame");
  
  frame->SyncAll();
  frame->SyncCloseAll(0);
  
  CPPUNIT_ASSERT( frame->TogglePane("FINDBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("FINDBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("OPTIONSBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("OPTIONSBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("TOOLBAR"));
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("TOOLBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("VIBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("VIBAR").IsShown());
  
  CPPUNIT_ASSERT(!frame->TogglePane("XXXXBAR"));
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("XXXXBAR").IsOk());
}
