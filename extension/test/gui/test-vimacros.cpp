////////////////////////////////////////////////////////////////////////////////
// Name:      test-vimacros.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vimacros.h>
#include <wx/extension/stc.h>
#include "test.h"

#define ESC "\x1b"

void wxExGuiTestFixture::testViMacros()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello");
  wxExVi* vi = &stc->GetVi();
  
  wxExViMacros macros;
  
  // Load, save document is last test, to be able to check contents.
  CPPUNIT_ASSERT(!macros.GetFileName().GetFullPath().empty());
  CPPUNIT_ASSERT( wxExViMacros::LoadDocument());
  
  CPPUNIT_ASSERT(!macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecording());
  
  macros.StartRecording("a");
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT( macros.IsRecording());
  CPPUNIT_ASSERT(!macros.IsRecorded("a"));
  
  macros.StopRecording();
  CPPUNIT_ASSERT(!macros.IsRecording());
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecorded("a")); // still no macro
  CPPUNIT_ASSERT( macros.GetMacro().empty());
  
  macros.StartRecording("a");
  macros.Record("a");
  macros.Record("test");
  macros.Record(ESC);
  macros.StopRecording();
  
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecording());
  CPPUNIT_ASSERT( macros.IsRecorded("a"));
  CPPUNIT_ASSERT( macros.GetMacro() == "a");
  
  CPPUNIT_ASSERT(!macros.IsRecorded("b"));
  
  stc->SetText("");
  CPPUNIT_ASSERT( macros.Playback(vi, "a"));
  CPPUNIT_ASSERT( macros.Get("a").front() == "a");
  CPPUNIT_ASSERT( stc->GetText().Contains("test"));
  stc->SetText("");
  CPPUNIT_ASSERT(!macros.Playback(vi, "a", 0));
  CPPUNIT_ASSERT(!macros.Playback(vi, "a", -8));
  CPPUNIT_ASSERT(!stc->GetText().Contains("test"));
  CPPUNIT_ASSERT( macros.Playback(vi, "a", 10));
  CPPUNIT_ASSERT( stc->GetText().Contains("testtesttesttest"));
  
  CPPUNIT_ASSERT(!macros.Playback(vi, "b"));
  
  CPPUNIT_ASSERT(!macros.Get().empty());
  
  // Test append to macro.
  CPPUNIT_ASSERT( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("w");
  macros.Record("/test");
  macros.StopRecording();
  
  CPPUNIT_ASSERT(!macros.IsRecorded("A"));
  CPPUNIT_ASSERT( macros.Get("a").front() == "a");
  
  // Test recursive macro.
  CPPUNIT_ASSERT( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("@");
  macros.Record("a");
  macros.StopRecording();
  
  CPPUNIT_ASSERT(!macros.Playback(vi, "a"));
  
  // Test variables.
  //CPPUNIT_ASSERT(!macros.Expand(vi, "xxx"));

  // Test all builtin macro variables.
  CPPUNIT_ASSERT( macros.Expand(vi, "Cb"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Cc"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Ce"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Cl"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Created"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Date"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Datetime"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Filename"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Fullpath"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Nl"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Path"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Time"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Year"));
  
  // Test environment macro variables.
  CPPUNIT_ASSERT( macros.Expand(vi, "Home"));

  // Test input macro variables.
  // Next requires input...    
  //  CPPUNIT_ASSERT( macros.Expand(vi, "author"));

  // Test template macro variables.
  //wxString value;
  //CPPUNIT_ASSERT( macros.Expand(vi, "cht", value));
  //CPPUNIT_ASSERT( value.Contains("Template example"));

  // So save as last test.
  CPPUNIT_ASSERT( wxExViMacros::SaveDocument());
  
  CPPUNIT_ASSERT(!macros.IsModified());
  
  // A second save is not necessary.
  CPPUNIT_ASSERT(!macros.SaveDocument());
  
  // Test registers.
  CPPUNIT_ASSERT(!macros.GetRegister('a').empty());
  CPPUNIT_ASSERT( macros.GetRegister('z').empty());
  CPPUNIT_ASSERT(!macros.GetRegisters().empty());
  CPPUNIT_ASSERT( macros.Get("z").empty());
  macros.SetRegister('z', "hello z");
  CPPUNIT_ASSERT(!macros.GetRegister('z').empty());
  CPPUNIT_ASSERT(!macros.Get("z").empty());
  CPPUNIT_ASSERT( macros.GetRegister('z') == "hello z");
  macros.SetRegister('Z', " and more");
  CPPUNIT_ASSERT( macros.GetRegister('Z').empty());
  CPPUNIT_ASSERT( macros.GetRegister('z') == "hello z and more");
}
