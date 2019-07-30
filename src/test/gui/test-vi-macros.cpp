////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-macros.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vi-macros.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vi-macros-mode.h>
#include "test.h"

#define ESC "\x1b"

TEST_CASE("wex::vi_macros")
{
  wex::stc* stc = new wex::stc(std::string("hello"));
  wex::test::add_pane(frame(), stc);
  wex::vi* vi = &stc->get_vi();

  wex::vi_macros macros;
  
  REQUIRE(!macros.get_filename().empty());
  REQUIRE( wex::vi_macros::load_document());
  REQUIRE( macros.size() > 0);
  REQUIRE(!macros.get_abbreviations().empty());
  
  REQUIRE(!macros.is_modified());
  REQUIRE(!macros.mode()->is_recording());
  
  macros.mode()->transition("qa");
  REQUIRE( macros.is_modified());
  REQUIRE( macros.mode()->is_recording());
  
  macros.mode()->transition("q");
  REQUIRE(!macros.mode()->is_recording());
  REQUIRE( macros.is_modified());
  REQUIRE(!macros.is_recorded("a")); // still no macro
  REQUIRE(!macros.is_recorded_macro("a"));
  REQUIRE( macros.get_macro().empty());
  
  macros.mode()->transition("qa");
  macros.record("a");
  macros.record("test");
  macros.record(ESC);
  macros.mode()->transition("q");

  REQUIRE( macros.is_modified());
  REQUIRE(!macros.mode()->is_recording());
  REQUIRE( macros.is_recorded("a"));
  REQUIRE( macros.starts_with("a"));
  REQUIRE(!macros.starts_with("xx"));
  REQUIRE( macros.is_recorded_macro("a"));
  REQUIRE( macros.get_macro() == "a");
  REQUIRE( macros.get("a").front() == "a");
  REQUIRE(!macros.is_recorded("b"));

  REQUIRE(!macros.get_keys_map().empty());
  macros.set_key_map("4", "www");
  
  stc->set_text("");
  REQUIRE( macros.mode()->transition("@a", vi));

  REQUIRE( stc->GetText() == "test");
  stc->set_text("");
  REQUIRE(!macros.mode()->transition("@a", vi, true, 0));
  REQUIRE(!macros.mode()->transition("@a", vi, true, -8));
  REQUIRE(!stc->GetText().Contains("test"));
  REQUIRE( macros.mode()->transition("@a", vi, true, 10));
  REQUIRE( stc->GetText().Contains("testtesttesttest"));
  
  REQUIRE( macros.mode()->transition("@b", vi) == 2);
  
  REQUIRE(!macros.get().empty());

  // Test append to macro.
  macros.mode()->transition("qA", vi);
  macros.record("w");
  macros.record("/test");
  macros.mode()->transition("q", vi);
  
  REQUIRE(!macros.is_recorded("A"));
  REQUIRE( macros.get("a").front() == "a");
  
  // Test recursive macro.
  macros.mode()->transition("qA", vi);
  macros.record("@");
  macros.record("a");
  macros.mode()->transition("q", vi);
  
  REQUIRE( macros.mode()->transition("@a", vi) );
  
  // Test all builtin macro variables.
  for (auto& builtin : get_builtin_variables())
  {
    CAPTURE( builtin );
    REQUIRE( macros.mode()->transition("@" + builtin + "@", vi));
  }

  std::string expanded;

  REQUIRE( wex::vi_macros::mode()->expand(vi, wex::variable(), expanded));

#ifdef __UNIX__
  // Test all environment macro variables.
  for (auto& env : std::vector<std::string> {"HOME","PWD"})
  {
    REQUIRE( wex::vi_macros::mode()->transition("@" + env, vi));
  }
#endif
  
  // Test input macro variables (requires input).
  // Test template macro variables (requires input).

  // So save as last test.
  REQUIRE( wex::vi_macros::save_document());
  
  REQUIRE(!macros.is_modified());
  
  // A second save is not necessary.
  REQUIRE(!macros.save_document());
  
  // Test registers.
  REQUIRE(!macros.get_register('a').empty());
  REQUIRE(!macros.registers().empty());
  REQUIRE( macros.set_register('z', "hello z"));
  REQUIRE(!macros.get("z").empty());
  REQUIRE( macros.get_register('z') == "hello z");
  REQUIRE( macros.set_register('Z', " and more"));
  REQUIRE( macros.get_register('Z').empty());
  REQUIRE( macros.get_register('z') == "hello z and more");
  REQUIRE(!macros.set_register('\x05', "hello z"));
  REQUIRE( macros.set_register('*', "clipboard"));
  REQUIRE( macros.set_register('_', "blackhole"));
  REQUIRE( macros.get_register('_').empty());
  
  // Test abbreviations.
  for (auto& abbrev : get_abbreviations())
  {
    macros.set_abbreviation(abbrev.first, abbrev.second);
    
    const auto& it = macros.get_abbreviations().find(abbrev.first);
            
    if (it != macros.get_abbreviations().end())
    {
      REQUIRE( abbrev.second == it->second);
    }
  }
  
  REQUIRE( macros.get_abbreviations().size() == get_abbreviations().size() + 1);
  REQUIRE( macros.is_modified());
  REQUIRE( wex::vi_macros::save_document());
}
