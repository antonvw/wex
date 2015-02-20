////////////////////////////////////////////////////////////////////////////////
// Name:      test-interruptable.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/interruptable.h>
#include "test.h"

void TestFixture::testInterruptable()
{
  wxExInterruptable interruptable;
  
  CPPUNIT_ASSERT(!interruptable.Running());
  CPPUNIT_ASSERT(!interruptable.Cancelled());
  CPPUNIT_ASSERT(!interruptable.Cancel());
  
  interruptable.Start();
  CPPUNIT_ASSERT( interruptable.Running());
  CPPUNIT_ASSERT(!interruptable.Cancelled());
  
  interruptable.Stop();
  CPPUNIT_ASSERT(!interruptable.Running());
  CPPUNIT_ASSERT(!interruptable.Cancelled());
  CPPUNIT_ASSERT(!interruptable.Cancel());
  
  interruptable.Start();
  CPPUNIT_ASSERT( interruptable.Cancel());
  CPPUNIT_ASSERT(!interruptable.Running());
  CPPUNIT_ASSERT( interruptable.Cancelled());
}
