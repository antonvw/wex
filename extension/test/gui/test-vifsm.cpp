////////////////////////////////////////////////////////////////////////////////
// Name:      test-vifsm.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vifsm.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>
#include "test.h"

#define ESC "\x1b"

void fixture::testViFSM()
{
  wxExSTC* stc = new wxExSTC(m_Frame);
  
  wxExViFSM fsm(&stc->GetVi(), 
    [=](const std::string& command){;},
    [=](const std::string& command){;});
  
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT(!fsm.Transition("x"));
  CPPUNIT_ASSERT(!fsm.Transition("y"));
  
  CPPUNIT_ASSERT( fsm.Transition("i"));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT(!fsm.Transition("i"));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_INSERT);
  
  CPPUNIT_ASSERT( fsm.Transition(ESC));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_NORMAL);
  
  CPPUNIT_ASSERT( fsm.Transition("cc"));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_INSERT);
  
  CPPUNIT_ASSERT( fsm.Transition(ESC));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_NORMAL);
  
  stc->SetReadOnly(true);
  CPPUNIT_ASSERT( fsm.Transition("i"));
  CPPUNIT_ASSERT( fsm.State() == wxExVi::MODE_NORMAL);
  
  wxExViFSMEntry entry(0, 1, 2, [=](const std::string& command){;});
  CPPUNIT_ASSERT( entry.State() == 0);
  CPPUNIT_ASSERT( entry.Action() == 1);
  CPPUNIT_ASSERT( entry.Next("test") == 2);
}
