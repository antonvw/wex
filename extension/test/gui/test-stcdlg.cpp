////////////////////////////////////////////////////////////////////////////////
// Name:      test-stcdlg.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stcdlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include "test.h"

void fixture::testSTCEntryDialog()
{
  wxExSTCEntryDialog dlg1(m_Frame, "hello", "testing");
  CPPUNIT_ASSERT( dlg1.GetText() == "testing");
  //CPPUNIT_ASSERT( dlg1.GetTextRaw() == "testing");
  CPPUNIT_ASSERT( dlg1.SetLexer("cpp"));
  CPPUNIT_ASSERT(!dlg1.SetLexer("xxx"));
  
  wxExSTCEntryDialog dlg2(
    m_Frame, 
      "hello", 
      "testing",
      "hello again",
      wxOK,
      true);
  CPPUNIT_ASSERT(!dlg2.GetText().empty());
  CPPUNIT_ASSERT( dlg2.GetTextRaw().length() > 0);
  CPPUNIT_ASSERT( dlg2.GetSTCShell() != NULL);
  CPPUNIT_ASSERT( dlg2.GetSTCShell()->GetPrompt() == "testing");
}
