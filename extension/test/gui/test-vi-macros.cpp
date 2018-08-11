////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi-macros.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi-macros-mode.h>
#include "test.h"

#define ESC "\x1b"

TEST_CASE("wxExViMacros")
{
  wxExSTC* stc = new wxExSTC(std::string("hello"));
  AddPane(GetFrame(), stc);
  wxExVi* vi = &stc->GetVi();

  wxExViMacros macros;
  
  REQUIRE(!macros.GetFileName().Path().empty());
  REQUIRE( wxExViMacros::LoadDocument());
  REQUIRE( macros.GetCount() > 0);
  REQUIRE(!macros.GetAbbreviations().empty());
  
  REQUIRE(!macros.IsModified());
  REQUIRE(!macros.Mode()->IsRecording());
  
  macros.Mode()->Transition("qa");
  REQUIRE( macros.IsModified());
  REQUIRE( macros.Mode()->IsRecording());
  
  macros.Mode()->Transition("q");
  REQUIRE(!macros.Mode()->IsRecording());
  REQUIRE( macros.IsModified());
  REQUIRE(!macros.IsRecorded("a")); // still no macro
  REQUIRE(!macros.IsRecordedMacro("a"));
  REQUIRE( macros.GetMacro().empty());
  
  macros.Mode()->Transition("qa");
  macros.Record("a");
  macros.Record("test");
  macros.Record(ESC);
  macros.Mode()->Transition("q");

  REQUIRE( macros.IsModified());
  REQUIRE(!macros.Mode()->IsRecording());
  REQUIRE( macros.IsRecorded("a"));
  REQUIRE( macros.StartsWith("a"));
  REQUIRE(!macros.StartsWith("xx"));
  REQUIRE( macros.IsRecordedMacro("a"));
  REQUIRE( macros.GetMacro() == "a");
  REQUIRE( macros.Get("a").front() == "a");
  REQUIRE(!macros.IsRecorded("b"));

  REQUIRE(!macros.GetKeysMap().empty());
  macros.SetKeyMap("4", "www");
  
  stc->SetText("");
  REQUIRE( macros.Mode()->Transition("@a", vi));

  REQUIRE( stc->GetText() == "test");
  stc->SetText("");
  REQUIRE(!macros.Mode()->Transition("@a", vi, true, 0));
  REQUIRE(!macros.Mode()->Transition("@a", vi, true, -8));
  REQUIRE(!stc->GetText().Contains("test"));
  REQUIRE( macros.Mode()->Transition("@a", vi, true, 10));
  REQUIRE( stc->GetText().Contains("testtesttesttest"));
  
  REQUIRE( macros.Mode()->Transition("@b", vi) == 2);
  
  REQUIRE(!macros.Get().empty());

  // Test append to macro.
  macros.Mode()->Transition("qA", vi);
  macros.Record("w");
  macros.Record("/test");
  macros.Mode()->Transition("q", vi);
  
  REQUIRE(!macros.IsRecorded("A"));
  REQUIRE( macros.Get("a").front() == "a");
  
  // Test recursive macro.
  macros.Mode()->Transition("qA", vi);
  macros.Record("@");
  macros.Record("a");
  macros.Mode()->Transition("q", vi);
  
  REQUIRE( macros.Mode()->Transition("@a", vi) );
  
  // Test all builtin macro variables.
  for (auto& builtin : GetBuiltinVariables())
  {
    CAPTURE( builtin );
    REQUIRE( macros.Mode()->Transition("@" + builtin + "@", vi));
  }

  std::string expanded;

  REQUIRE(!wxExViMacros::Mode()->Expand(vi, wxExVariable(), expanded));

#ifdef __UNIX__
  // Test all environment macro variables.
  for (auto& env : std::vector<std::string> {"HOME","PWD"})
  {
    REQUIRE( wxExViMacros::Mode()->Transition("@" + env, vi));
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
