////////////////////////////////////////////////////////////////////////////////
// Name:      test-vimacros.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExViMacros")
{
  wxExSTC* stc = new wxExSTC(GetFrame(), std::string("hello"));
  AddPane(GetFrame(), stc);
  wxExVi* vi = &stc->GetVi();

  wxExViMacros macros;
  
  // Load, save document is last test, to be able to check contents.
  REQUIRE(!macros.GetFileName().GetFullPath().empty());
  REQUIRE( wxExViMacros::LoadDocument());
  REQUIRE( macros.GetCount() > 0);
  REQUIRE(!macros.GetAbbreviations().empty());
  
  REQUIRE(!macros.IsModified());
  REQUIRE(!macros.IsRecording());
  
  macros.StartRecording("a");
  REQUIRE( macros.IsModified());
  REQUIRE( macros.IsRecording());
  
  macros.StopRecording();
  REQUIRE(!macros.IsRecording());
  REQUIRE( macros.IsModified());
  REQUIRE(!macros.IsRecorded("a")); // still no macro
  REQUIRE(!macros.IsRecordedMacro("a"));
  REQUIRE( macros.GetMacro().empty());
  
  macros.StartRecording("a");
  macros.Record("a");
  macros.Record("test");
  macros.Record(ESC);
  macros.StopRecording();

  REQUIRE( macros.IsModified());
  REQUIRE(!macros.IsRecording());
  REQUIRE( macros.IsRecorded("a"));
  REQUIRE( macros.StartsWith("a"));
  REQUIRE(!macros.StartsWith("xx"));
  REQUIRE( macros.IsRecordedMacro("a"));
  REQUIRE( macros.GetMacro() == "a");
  REQUIRE( macros.Get("a").front() == "a");
  REQUIRE(!macros.IsRecorded("b"));
  
  stc->SetText("");
  REQUIRE(!macros.IsPlayback());
  REQUIRE( macros.Playback(vi, "a"));
  REQUIRE(!macros.IsPlayback());

  REQUIRE( stc->GetText() == "test");
  stc->SetText("");
  REQUIRE(!macros.Playback(vi, "a", 0));
  REQUIRE(!macros.Playback(vi, "a", -8));
  REQUIRE(!stc->GetText().Contains("test"));
  REQUIRE( macros.Playback(vi, "a", 10));
  REQUIRE( stc->GetText().Contains("testtesttesttest"));
  
  REQUIRE(!macros.Playback(vi, "b"));
  
  REQUIRE(!macros.Get().empty());

  // Test append to macro.
  REQUIRE( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("w");
  macros.Record("/test");
  macros.StopRecording();
  
  REQUIRE(!macros.IsRecorded("A"));
  REQUIRE( macros.Get("a").front() == "a");
  
  // Test recursive macro.
  REQUIRE( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("@");
  macros.Record("a");
  macros.StopRecording();
  
  REQUIRE(!macros.Playback(vi, "a"));
  
  // Test all builtin macro variables.
  for (auto& builtin : GetBuiltinVariables())
  {
    REQUIRE( macros.Expand(vi, builtin));
  }

  std::string expanded;
  REQUIRE(!wxExViMacros::ExpandTemplate(vi, wxExVariable(), expanded));

#ifdef __UNIX__
  // Test all environment macro variables.
  for (auto& env : std::vector<std::string> {"HOME","PWD"})
  {
    REQUIRE( macros.Expand(vi, env));
  }
#endif
  
  // Test input macro variables (requires input).
  // Test template macro variables (requires input).

  // So save as last test.
  REQUIRE( wxExViMacros::SaveDocument());
  
  REQUIRE(!macros.IsModified());
  
  // A second save is not necessary.
  REQUIRE(!macros.SaveDocument());
  
  // Test registers.
  REQUIRE(!macros.GetRegister('a').empty());
  REQUIRE(!macros.GetRegisters().empty());
  REQUIRE( macros.SetRegister('z', "hello z"));
  REQUIRE(!macros.Get("z").empty());
  REQUIRE( macros.GetRegister('z') == "hello z");
  REQUIRE( macros.SetRegister('Z', " and more"));
  REQUIRE( macros.GetRegister('Z').empty());
  REQUIRE( macros.GetRegister('z') == "hello z and more");
  REQUIRE(!macros.SetRegister('\x05', "hello z"));
  REQUIRE( macros.SetRegister('*', "clipboard"));
  REQUIRE( macros.SetRegister('_', "blackhole"));
  REQUIRE( macros.GetRegister('_').empty());
  
  // Test abbreviations.
  for (auto& abbrev : GetAbbreviations())
  {
    macros.SetAbbreviation(abbrev.first, abbrev.second);
    
    const auto& it = macros.GetAbbreviations().find(abbrev.first);
            
    if (it != macros.GetAbbreviations().end())
    {
      REQUIRE( abbrev.second == it->second);
    }
  }
  
  REQUIRE( macros.GetAbbreviations().size() == GetAbbreviations().size() + 1);
  REQUIRE( macros.IsModified());
  REQUIRE( wxExViMacros::SaveDocument());
}
