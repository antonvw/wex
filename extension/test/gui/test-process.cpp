////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/process.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include "test.h"

void wxExGuiTestFixture::testProcess()
{
  wxExProcess process;
  
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT( process.GetOutput().empty());
  CPPUNIT_ASSERT(!process.HasStdError());
  CPPUNIT_ASSERT(!process.IsRunning());
  
  process.ConfigDialog(m_Frame, "test process", false);
  
  // Test wxEXEC_SYNC process.
  /*
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
  
  CPPUNIT_ASSERT(!process.IsRunning());
  CPPUNIT_ASSERT( process.IsSelected());
  CPPUNIT_ASSERT( process.GetSTC() != NULL);
  CPPUNIT_ASSERT(!process.GetSTC()->GetText().empty());
  CPPUNIT_ASSERT( process.Kill() == wxKILL_NO_PROCESS);
  
  process.ShowOutput();

  // Repeat last wxEXEC_SYNC process.
  // Currently dialog might be cancelled, so do not check return value.
  //  process.Execute("", wxEXEC_SYNC);
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
*/
  // TODO:
  // Test invalid wxEXEC_SYNC process.
  
  // Test wxEXEC_ASYNC process.
  // wxExecute hangs for wxEXEC_ASYNC
  CPPUNIT_ASSERT( process.Execute("bash"));
  CPPUNIT_ASSERT( process.IsRunning());
  wxExSTCShell* shell = process.GetShell();  
  
  CPPUNIT_ASSERT( shell != NULL);
  
  // Test commands entered in shell.
  const wxString cwd = wxGetCwd();
  
  shell->ProcessChar('c');
  shell->ProcessChar('d');
  shell->ProcessChar(' ');
  shell->ProcessChar('~');
  shell->ProcessChar('\r');
  shell->ProcessChar('p');
  shell->ProcessChar('w');
  shell->ProcessChar('d');
  shell->ProcessChar('\r');
  
  CPPUNIT_ASSERT( shell->GetText().Contains("home"));
//  CPPUNIT_ASSERT( cwd != wxGetCwd());

  // Test invalid wxEXEC_ASYNC process (TODO: but it is started??).
  CPPUNIT_ASSERT( process.Execute("xxxx"));
  CPPUNIT_ASSERT(!process.GetError());
  // The output is not touched by the async process, so if it was not empty,
  // it still is not empty.
//  CPPUNIT_ASSERT(!process.GetOutput().empty());
  
  // TODO:
  // Repeat last process (wxEXEC_ASYNC).
  
  // Go back to where we were, necessary for other tests.
  wxSetWorkingDirectory(cwd);
}
