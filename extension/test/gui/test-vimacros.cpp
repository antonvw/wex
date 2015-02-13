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
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

#define ESC "\x1b"

void wxExGuiTestFixture::testViMacros()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello");
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
  
  // Test all builtin macro variables.
  for (auto& builtin : m_BuiltinVariables)
  {
    CPPUNIT_ASSERT( macros.Expand(vi, builtin));
  }

  // Test all environment macro variables.
  const std::vector<std::string> envs{"HOME","PWD"};
  
  for (auto& env : envs)
  {
    CPPUNIT_ASSERT( macros.Expand(vi, env));
  }
  
  // Test input macro variables (requires input).
  // Test template macro variables (requires input).

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
  
  // Test abbreviations.
  CPPUNIT_ASSERT( macros.GetAbbreviations().empty());
  
  for (auto& abbrev : m_Abbreviations)
  {
    macros.SetAbbreviation(abbrev.first, abbrev.second);
    
    const auto& it = macros.GetAbbreviations().find(abbrev.first);
            
    if (it != macros.GetAbbreviations().end())
    {
      CPPUNIT_ASSERT( abbrev.second == it->second);
    }
  }
  
  CPPUNIT_ASSERT( macros.GetAbbreviations().size() == m_Abbreviations.size());
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT( wxExViMacros::SaveDocument());
}
