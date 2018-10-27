////////////////////////////////////////////////////////////////////////////////
// Name:      test-addressrange.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/addressrange.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vi-macros.h>
#include "test.h"

TEST_CASE("wex::addressrange")
{
  wex::stc* stc = new wex::stc();
  AddPane(GetFrame(), stc);

  stc->SetText("hello\nhello1\nhello2");
  wex::ex* ex = new wex::ex(stc);
  ex->MarkerAdd('x', 1);
  ex->MarkerAdd('y', 2);
  ex->GetMacros().SetRegister('*', "ls");
  stc->GotoLine(2);

  // Test valid ranges when no selection is active.
  REQUIRE( wex::addressrange(ex).is_ok());
  REQUIRE( wex::addressrange(ex, -1).is_ok());
  REQUIRE( wex::addressrange(ex, 5).is_ok());
  REQUIRE( wex::addressrange(ex, "%").is_ok());
  REQUIRE( wex::addressrange(ex, "*").is_ok());
  REQUIRE( wex::addressrange(ex, ".").is_ok());
  REQUIRE( wex::addressrange(ex, "1,2").is_ok());
  REQUIRE( wex::addressrange(ex, "/1/,/2/").is_ok());
  REQUIRE( wex::addressrange(ex, "?1?,?2?").is_ok());
  
  // Test invalid ranges when no selection is active.
  REQUIRE(!wex::addressrange(ex, 0).is_ok());
  REQUIRE(!wex::addressrange(ex, "0").is_ok());
  REQUIRE(!wex::addressrange(ex, "x").is_ok());
  REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
  REQUIRE(!wex::addressrange(ex, "x,3").Delete());
  REQUIRE(!wex::addressrange(ex, "3,x").Escape("ls"));
  REQUIRE(!wex::addressrange(ex, "3,x").ShiftRight());
  REQUIRE(!wex::addressrange(ex, "3,!").is_ok());
  REQUIRE(!wex::addressrange(ex, "3,@").Move(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "1,2").Move(wex::address(ex, "x")));
  REQUIRE(!wex::addressrange(ex, "1,3").Move(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "3,@").Copy(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "3,x").Write("flut"));
  REQUIRE(!wex::addressrange(ex, " ,").Yank());
  REQUIRE(!wex::addressrange(ex, "'<,'>").is_ok());
  REQUIRE(!wex::addressrange(ex, "/xx/,/2/").is_ok());
  REQUIRE(!wex::addressrange(ex, "?2?,?1?").is_ok());
  
  stc->SelectAll();
  
  // Test valid ranges when selection is active.
  REQUIRE( wex::addressrange(ex, 5).is_ok());
  REQUIRE( wex::addressrange(ex, "'<,'>").is_ok());
  
  // Test invalid ranges when selection is active.
  REQUIRE(!wex::addressrange(ex, 0).is_ok());
  REQUIRE(!wex::addressrange(ex, "0").is_ok());
  REQUIRE(!wex::addressrange(ex, "x").is_ok());
  REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
  
  stc->SelectNone();
  
  REQUIRE( wex::addressrange(ex, "1,3").Delete());
  REQUIRE( wex::addressrange(ex, "1,3").Delete());
  
  // Test implementation.  
  const std::string contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  
  // Test Change.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wex::addressrange(ex, 4).Change("changed"));
  REQUIRE( stc->GetText().Contains("changed"));
  
  // Test Copy.
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "1,2").Copy(wex::address(ex, "$")));
  REQUIRE( stc->GetLineCount() == 10);
  
  // Test Delete.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(1);
  REQUIRE( wex::addressrange(ex, 5).Delete());
  REQUIRE( stc->GetLineCount() == 3);
  REQUIRE(!wex::addressrange(ex, 0).Delete());
  REQUIRE( stc->GetLineCount() == 3);
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->SelectAll();
  REQUIRE( wex::addressrange(ex, "'<,'>").Delete());
  REQUIRE( stc->GetLineCount() == 1);
  stc->SelectNone();
  
  // Test Escape.
#ifdef __UNIX__
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "%").Escape("uniq"));
  REQUIRE( stc->GetLineCount() == 5);
  REQUIRE( wex::addressrange(ex).Escape("ls -l"));
  REQUIRE( wex::addressrange::GetProcess() != nullptr);
  REQUIRE( wex::addressrange(ex).Escape("ls `pwd`"));
  REQUIRE( wex::addressrange(ex).Escape("ls \x12*"));
  REQUIRE( wex::addressrange(ex).Escape("ls  `echo \x12*`"));
#endif
  
#ifdef __UNIX__
  // Test Global and Global inverse.
  for (bool b : { false, true })
  {
    stc->SetText(contents);
    REQUIRE(!wex::addressrange(ex, 5).Global(std::string(), b));
    REQUIRE(!wex::addressrange(ex, 5).Global("XXX", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/xx/p", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/xx/p#", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/xx/g", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/a/s/a/XX", b));
    REQUIRE( wex::addressrange(ex, 5).Global("/b/s/b/XX|s/c/yy", b));
  }
#endif
  
  // Test Shift.
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, 5).ShiftRight());
  REQUIRE( wex::addressrange(ex, 5).ShiftLeft());
  
  // Test is_ok.
  // See above.
  
  // Test Join.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wex::addressrange(ex, "%").Join());
  REQUIRE( stc->GetText().Contains("a"));
  REQUIRE( stc->GetLineCount() == 1);
  
  // Test Move.
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "1,2").Move(wex::address(ex, "$")));
  REQUIRE( stc->GetLineCount() == 8);
  
  // Test Print.
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, 5).Print());
  
  // Test Sort.
  REQUIRE( wex::addressrange(ex, "1").Sort());
  REQUIRE(!wex::addressrange(ex, "1").Sort("x"));
  REQUIRE( wex::addressrange(ex, "1").Sort("u"));
  REQUIRE( wex::addressrange(ex, "1").Sort("r"));
  REQUIRE( wex::addressrange(ex, "1").Sort("ur"));
  
  // Test Substitute.
  stc->SetText(contents);
  stc->GotoLine(1);
  REQUIRE( wex::addressrange(ex, "%").Substitute("/tiger//"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, "%").
    Substitute("/tiger/\\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, "%").
    Substitute("/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE( stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  REQUIRE(!stc->GetText().Contains("\\L"));
  REQUIRE(!stc->GetText().Contains("\\0"));
  
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, "%").Substitute("/tiger/lion/"));
  REQUIRE( stc->GetText().Contains("lion"));
    
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, "%").Substitute("", '&'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, "%").Substitute("", '~'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->SetText("special char \\ present");
  REQUIRE( wex::addressrange(ex, "%").Substitute("/\\\\//"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char / present");
  REQUIRE( wex::addressrange(ex, "%").Substitute("/\\///"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char ' present");
  REQUIRE( wex::addressrange(ex, "%").Substitute("/'//"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  // Test Substitute and flags.
  REQUIRE(!wex::addressrange(ex, "1").Substitute("//y"));
  REQUIRE(!wex::addressrange(ex, "0").Substitute("/x/y"));
  REQUIRE( wex::addressrange(ex, "2").Substitute("/x/y/f"));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("/x/y"));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("/x/y/i"));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("/x/y/f"));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("/x/y/g"));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("g", '&'));
  REQUIRE( wex::addressrange(ex, "1,2").Substitute("g", '~'));
  REQUIRE(!wex::addressrange(ex, "1,2").Substitute("g", 'x'));

  // Test Write.
  stc->SetText(contents);
  REQUIRE( wex::addressrange(ex, 5).Write("sample.txt"));
  REQUIRE( remove("sample.txt") == 0);
  
  // Test Yank.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(0);
  REQUIRE( wex::addressrange(ex, 2).Yank());
  stc->SelectNone();
  stc->AddText(stc->GetVi().GetMacros().GetRegister('0'));
  REQUIRE( stc->GetLineCount() == 10);
  REQUIRE( wex::addressrange(ex, -2).Delete());
  stc->GotoLine(0);
  REQUIRE( wex::addressrange(ex, -2).Delete());
}
