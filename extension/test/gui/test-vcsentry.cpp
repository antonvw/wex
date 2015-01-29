////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcsentry.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcsentry.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include "test.h"

void wxExGuiTestFixture::testVCSEntry()
{
  wxExVCSEntry test;
  
  CPPUNIT_ASSERT( test.GetCommands() == 1);
  
  wxExVCSEntry test2;
  
  CPPUNIT_ASSERT( test2.GetCommands() == 1);
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT(!test.AdminDirIsTopLevel());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
  
  CPPUNIT_ASSERT( test.ShowDialog(
    m_Frame,
    "vcs",
    false) == wxID_CANCEL);
    
  test.ShowOutput();
  
  wxMenu menu;
  CPPUNIT_ASSERT( test.BuildMenu(0, &menu) == 0);
  
  // This should have no effect.  
  CPPUNIT_ASSERT(!test.SetCommand(5));
  CPPUNIT_ASSERT(!test.SetCommand(ID_EDIT_VCS_LOWEST));
  CPPUNIT_ASSERT(!test.SetCommand(ID_VCS_LOWEST));
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
}
