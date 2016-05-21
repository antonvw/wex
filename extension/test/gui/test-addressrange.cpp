////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/addressrange.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

TEST_CASE("wxExAddressRange", "[stc][vi]")
{
  wxExSTC* stc = GetSTC();
  stc->SetText("hello\nhello1\nhello2");
  wxExEx* ex = new wxExEx(stc);
  ex->MarkerAdd('x', 1);
  ex->MarkerAdd('y', 2);
  ex->GetMacros().SetRegister('*', "ls");
  stc->GotoLine(2);

  // Test valid ranges when no selection is active.
  REQUIRE( wxExAddressRange(ex).IsOk());
  REQUIRE( wxExAddressRange(ex, -1).IsOk());
  REQUIRE( wxExAddressRange(ex, 5).IsOk());
  REQUIRE( wxExAddressRange(ex, "%").IsOk());
  REQUIRE( wxExAddressRange(ex, "*").IsOk());
  REQUIRE( wxExAddressRange(ex, ".").IsOk());
  REQUIRE( wxExAddressRange(ex, "1,2").IsOk());
  REQUIRE( wxExAddressRange(ex, "/1/,/2/").IsOk());
  REQUIRE( wxExAddressRange(ex, "?1?,?2?").IsOk());
  
  // Test invalid ranges when no selection is active.
  REQUIRE(!wxExAddressRange(ex, 0).IsOk());
  REQUIRE(!wxExAddressRange(ex, "0").IsOk());
  REQUIRE(!wxExAddressRange(ex, "x").IsOk());
  REQUIRE(!wxExAddressRange(ex, "x,3").IsOk());
  REQUIRE(!wxExAddressRange(ex, "x,3").Delete());
  REQUIRE(!wxExAddressRange(ex, "3,x").Escape("ls"));
  REQUIRE(!wxExAddressRange(ex, "3,x").Indent());
  REQUIRE(!wxExAddressRange(ex, "3,!").IsOk());
  REQUIRE(!wxExAddressRange(ex, "3,@").Move(wxExAddress(ex, "2")));
  REQUIRE(!wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "x")));
  REQUIRE(!wxExAddressRange(ex, "1,3").Move(wxExAddress(ex, "2")));
  REQUIRE(!wxExAddressRange(ex, "3,@").Copy(wxExAddress(ex, "2")));
  REQUIRE(!wxExAddressRange(ex, "3,x").Write("flut"));
  REQUIRE(!wxExAddressRange(ex, " ,").Yank());
  REQUIRE(!wxExAddressRange(ex, "'<,'>").IsOk());
  REQUIRE(!wxExAddressRange(ex, "/xx/,/2/").IsOk());
  REQUIRE(!wxExAddressRange(ex, "?2?,?1?").IsOk());
  
  stc->SelectAll();
  
  // Test valid ranges when selection is active.
  REQUIRE( wxExAddressRange(ex, 5).IsOk());
  REQUIRE( wxExAddressRange(ex, "'<,'>").IsOk());
  
  // Test invalid ranges when selection is active.
  REQUIRE(!wxExAddressRange(ex, 0).IsOk());
  REQUIRE(!wxExAddressRange(ex, "0").IsOk());
  REQUIRE(!wxExAddressRange(ex, "x").IsOk());
  REQUIRE(!wxExAddressRange(ex, "x,3").IsOk());
  
  stc->SelectNone();
  
  REQUIRE( wxExAddressRange(ex, "1,3").Delete());
  REQUIRE( wxExAddressRange(ex, "1,3").Delete());
  
  // Test implementation.  
  const wxString contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  
  // Test Change.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wxExAddressRange(ex, 4).Change("changed"));
  REQUIRE( stc->GetText().Contains("changed"));
  
  // Test Copy.
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wxExAddressRange(ex, "1,2").Copy(wxExAddress(ex, "$")));
  REQUIRE( stc->GetLineCount() == 10);
  
  // Test Delete.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(1);
  REQUIRE( wxExAddressRange(ex, 5).Delete());
  REQUIRE( stc->GetLineCount() == 3);
  REQUIRE(!wxExAddressRange(ex, 0).Delete());
  REQUIRE( stc->GetLineCount() == 3);
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->SelectAll();
  REQUIRE( wxExAddressRange(ex, "'<,'>").Delete());
  REQUIRE( stc->GetLineCount() == 1);
  stc->SelectNone();
  
  // Test Escape.
#ifdef __UNIX__
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wxExAddressRange(ex, "%").Escape("uniq"));
  REQUIRE( stc->GetLineCount() == 5);
  REQUIRE( wxExAddressRange(ex).Escape("ls -l"));
  REQUIRE( wxExAddressRange(ex).Escape("ls `pwd`"));
  REQUIRE( wxExAddressRange(ex).Escape("ls \x12*"));
  REQUIRE( wxExAddressRange(ex).Escape("ls  `echo \x12*`"));
  REQUIRE( wxExAddressRange::GetProcess() != nullptr);
#endif
  
  // Test Global and Global inverse.
  for (bool b : { false, true })
  {
    stc->SetText(contents);
    REQUIRE( wxExAddressRange(ex, 5).Global(wxEmptyString, b));
    REQUIRE( wxExAddressRange(ex, 5).Global("XXX", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/xx/p", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/xx/p#", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/xx/g", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/a/s/a/XX", b));
    REQUIRE( wxExAddressRange(ex, 5).Global("/b/s/b/XX|s/c/yy", b));
  }
  
  // Test Indent.
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, 5).Indent());
  REQUIRE( wxExAddressRange(ex, 5).Indent(false));
  
  // Test IsOk.
  // See above.
  
  // Test Join.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  REQUIRE( wxExAddressRange(ex, "%").Join());
  REQUIRE( stc->GetText().Contains("a"));
  INFO( std::to_string(stc->GetLineCount()));
  REQUIRE( stc->GetLineCount() == 1);
  
  // Test Move.
  stc->SetText(contents);
  REQUIRE( stc->GetLineCount() == 8);
  REQUIRE( wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "$")));
  REQUIRE( stc->GetLineCount() == 8);
  
  // Test Print.
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, 5).Print());
  
  // Test Sort.
  REQUIRE( wxExAddressRange(ex, "1").Sort());
  REQUIRE(!wxExAddressRange(ex, "1").Sort("x"));
  REQUIRE( wxExAddressRange(ex, "1").Sort("u"));
  REQUIRE( wxExAddressRange(ex, "1").Sort("r"));
  REQUIRE( wxExAddressRange(ex, "1").Sort("ur"));
  
  // Test Substitute.
  stc->SetText(contents);
  stc->GotoLine(1);
  REQUIRE( wxExAddressRange(ex, "%").
    Substitute("/tiger//"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, "%").
    Substitute("/tiger/\\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE(!stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, "%").
    Substitute("/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
  REQUIRE( stc->GetText().Contains("TIGER"));
  REQUIRE( stc->GetText().Contains("tiger"));
  REQUIRE(!stc->GetText().Contains("\\U"));
  REQUIRE(!stc->GetText().Contains("\\L"));
  REQUIRE(!stc->GetText().Contains("\\0"));
  
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, "%").Substitute("/tiger/lion/"));
  REQUIRE( stc->GetText().Contains("lion"));
    
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, "%").Substitute("", '&'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, "%").Substitute("", '~'));
  REQUIRE( stc->GetText().Contains("lion"));
  
  stc->SetText("special char \\ present");
  REQUIRE( wxExAddressRange(ex, "%").Substitute("/\\\\//"));
  INFO( stc->GetText().ToStdString() );
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char / present");
  REQUIRE( wxExAddressRange(ex, "%").Substitute("/\\///"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char ' present");
  REQUIRE( wxExAddressRange(ex, "%").Substitute("/'///"));
  REQUIRE( stc->GetText().Contains("char  present"));
  
  // Test Substitute and flags.
  REQUIRE(!wxExAddressRange(ex, "1").Substitute("//y"));
  REQUIRE(!wxExAddressRange(ex, "0").Substitute("/x/y"));
  REQUIRE(!wxExAddressRange(ex, "2").Substitute("/x/y/f"));
  REQUIRE( wxExAddressRange(ex, "1,2").Substitute("/x/y"));
  REQUIRE( wxExAddressRange(ex, "1,2").Substitute("/x/y/i"));
  REQUIRE(!wxExAddressRange(ex, "1,2").Substitute("/x/y/f"));
  REQUIRE( wxExAddressRange(ex, "1,2").Substitute("/x/y/g"));
  REQUIRE( wxExAddressRange(ex, "1,2").Substitute("g", '&'));
  REQUIRE( wxExAddressRange(ex, "1,2").Substitute("g", '~'));
  REQUIRE(!wxExAddressRange(ex, "1,2").Substitute("g", 'x'));

  // Test Write.
  stc->SetText(contents);
  REQUIRE( wxExAddressRange(ex, 5).Write("sample.txt"));
  REQUIRE( remove("sample.txt") == 0);
  
  // Test Yank.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(0);
  REQUIRE( wxExAddressRange(ex, 2).Yank());
  stc->SelectNone();
  stc->AddText(stc->GetVi().GetMacros().GetRegister('0'));
  REQUIRE( stc->GetLineCount() == 10);
  REQUIRE( wxExAddressRange(ex, -2).Delete());
  stc->GotoLine(0);
  REQUIRE( wxExAddressRange(ex, -2).Delete());
}
