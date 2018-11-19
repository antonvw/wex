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
  AddPane(frame(), stc);

  stc->set_text("hello\nhello1\nhello2");
  wex::ex* ex = new wex::ex(stc);
  ex->marker_add('x', 1);
  ex->marker_add('y', 2);
  ex->get_macros().set_register('*', "ls");
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
  REQUIRE(!wex::addressrange(ex, "x,3").erase());
  REQUIRE(!wex::addressrange(ex, "3,x").escape("ls"));
  REQUIRE(!wex::addressrange(ex, "3,x").shift_right());
  REQUIRE(!wex::addressrange(ex, "3,!").is_ok());
  REQUIRE(!wex::addressrange(ex, "3,@").move(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "1,2").move(wex::address(ex, "x")));
  REQUIRE(!wex::addressrange(ex, "1,3").move(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "3,@").copy(wex::address(ex, "2")));
  REQUIRE(!wex::addressrange(ex, "3,x").write("flut"));
  REQUIRE(!wex::addressrange(ex, " ,").yank());
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
  
  REQUIRE( wex::addressrange(ex, "1,3").erase());
  REQUIRE( wex::addressrange(ex, "1,3").erase());
  
  // Test implementation.  
  const std::string contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  
  // Test change.
  stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wex::addressrange(ex, 4).change("changed"));
  REQUIRE( stc->GetText().Contains("changed"));
  
  // Test copy.
  stc->set_text(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "1,2").copy(wex::address(ex, "$")));
  REQUIRE( stc->GetLineCount() == 10);
  
  // Test erase.
  stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(1);
  REQUIRE( wex::addressrange(ex, 5).erase());
  REQUIRE( stc->GetLineCount() == 3);
  REQUIRE(!wex::addressrange(ex, 0).erase());
  REQUIRE( stc->GetLineCount() == 3);
  stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
  stc->SelectAll();
  REQUIRE( wex::addressrange(ex, "'<,'>").erase());
  REQUIRE( stc->GetLineCount() == 1);
  stc->SelectNone();
  
  // Test escape.
#ifdef __UNIX__
  stc->set_text(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "%").escape("uniq"));
  REQUIRE( stc->GetLineCount() == 5);
  REQUIRE( wex::addressrange(ex).escape("ls -l"));
  REQUIRE( wex::addressrange::process() != nullptr);
  REQUIRE( wex::addressrange(ex).escape("ls `pwd`"));
  REQUIRE( wex::addressrange(ex).escape("ls \x12*"));
  REQUIRE( wex::addressrange(ex).escape("ls  `echo \x12*`"));
#endif
  
#ifdef __UNIX__
  // Test global and global inverse.
  for (bool b : { false, true })
  {
    stc->set_text(contents);
    REQUIRE(!wex::addressrange(ex, 5).global(std::string(), b));
    REQUIRE(!wex::addressrange(ex, 5).global("XXX", b));
    REQUIRE( wex::addressrange(ex, 5).global("/", b));
    REQUIRE( wex::addressrange(ex, 5).global("/xx/p", b));
    REQUIRE( wex::addressrange(ex, 5).global("/xx/p#", b));
    REQUIRE( wex::addressrange(ex, 5).global("/xx/g", b));
    REQUIRE( wex::addressrange(ex, 5).global("/a/s/a/XX", b));
    REQUIRE( wex::addressrange(ex, 5).global("/b/s/b/XX|s/c/yy", b));
  }
#endif
  
  // Test Shift.
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, 5).shift_right());
  REQUIRE( wex::addressrange(ex, 5).shift_left());
  
  // Test is_ok.
  // See above.
  
  // Test join.
  stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wex::addressrange(ex, "%").join());
  REQUIRE( stc->GetText().Contains("a"));
  REQUIRE( stc->GetLineCount() == 1);
  
  // Test move.
  stc->set_text(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wex::addressrange(ex, "1,2").move(wex::address(ex, "$")));
  REQUIRE( stc->GetLineCount() == 8);
  
  // Test print.
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, 5).print());
  
  // Test sort.
  REQUIRE( wex::addressrange(ex, "1").sort());
  REQUIRE(!wex::addressrange(ex, "1").sort("x"));
  REQUIRE( wex::addressrange(ex, "1").sort("u"));
  REQUIRE( wex::addressrange(ex, "1").sort("r"));
  REQUIRE( wex::addressrange(ex, "1").sort("ur"));
  
  // Test substitute.
  stc->set_text(contents);
  stc->GotoLine(1);
  REQUIRE( wex::addressrange(ex, "%").substitute("/tiger//"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, "%").
    substitute("/tiger/\\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, "%").
    substitute("/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE( stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  REQUIRE(!stc->GetText().Contains("\\L"));
  REQUIRE(!stc->GetText().Contains("\\0"));
  
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, "%").substitute("/tiger/lion/"));
  REQUIRE( stc->GetText().Contains("lion"));
    
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, "%").substitute("", '&'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, "%").substitute("", '~'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->set_text("special char \\ present");
  REQUIRE( wex::addressrange(ex, "%").substitute("/\\\\//"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->set_text("special char / present");
  REQUIRE( wex::addressrange(ex, "%").substitute("/\\///"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->set_text("special char ' present");
  REQUIRE( wex::addressrange(ex, "%").substitute("/'//"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  // Test substitute and flags.
  REQUIRE(!wex::addressrange(ex, "1").substitute("//y"));
  REQUIRE(!wex::addressrange(ex, "0").substitute("/x/y"));
  REQUIRE( wex::addressrange(ex, "2").substitute("/x/y/f"));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("/x/y"));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("/x/y/i"));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("/x/y/f"));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("/x/y/g"));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("g", '&'));
  REQUIRE( wex::addressrange(ex, "1,2").substitute("g", '~'));
  REQUIRE(!wex::addressrange(ex, "1,2").substitute("g", 'x'));

  // Test write.
  stc->set_text(contents);
  REQUIRE( wex::addressrange(ex, 5).write("sample.txt"));
  REQUIRE( remove("sample.txt") == 0);
  
  // Test Yank.
  stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(0);
  REQUIRE( wex::addressrange(ex, 2).yank());
  stc->SelectNone();
  stc->AddText(stc->get_vi().get_macros().get_register('0'));
  REQUIRE( stc->GetLineCount() == 10);
  REQUIRE( wex::addressrange(ex, -2).erase());
  stc->GotoLine(0);
  REQUIRE( wex::addressrange(ex, -2).erase());
}
