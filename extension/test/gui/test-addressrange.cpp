////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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

void fixture::testAddressRange()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello\nhello1\nhello2");
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLine(2);

  // Test valid ranges when no selection is active.
  CPPUNIT_ASSERT( wxExAddressRange(ex).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, -1).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "*").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, ".").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "/1/,/2/").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "?1?,?2?").IsOk());
  
  // Test invalid ranges when no selection is active.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").Delete());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Escape("ls"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Indent());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,!").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,@").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "x")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,3").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,@").Copy(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Write("flut"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, " ,").Yank());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "'<,'>").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "/xx/,/2/").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "?2?,?1?").IsOk());
  
  stc->SelectAll();
  
  // Test valid ranges when selection is active.
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "'<,'>").IsOk());
  
  // Test invalid ranges when selection is active.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").IsOk());
  
  stc->SelectNone();
  
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,3").Delete());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,3").Delete());
  
  // Test implementation.  
  const wxString contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  
  // Test Change.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, 4).Change("changed"));
  CPPUNIT_ASSERT( stc->GetText().Contains("changed"));
  
  // Test Copy.
  stc->SetText(contents);
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Copy(wxExAddress(ex, "$")));
  CPPUNIT_ASSERT( stc->GetLineCount() == 10);
  
  // Test Delete.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(1);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Delete());
  CPPUNIT_ASSERT( stc->GetLineCount() == 3);
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).Delete());
  CPPUNIT_ASSERT( stc->GetLineCount() == 3);
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->SelectAll();
  CPPUNIT_ASSERT( wxExAddressRange(ex, "'<,'>").Delete());
  CPPUNIT_ASSERT( stc->GetLineCount() == 1);
  stc->SelectNone();
  
  // Test Escape.
  stc->SetText(contents);
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Escape("uniq"));
  CPPUNIT_ASSERT( stc->GetLineCount() == 5);
  
  // Test Global and Global inverse.
  for (int i = 0; i < 2; i++)
  {
    stc->SetText(contents);
    CPPUNIT_ASSERT(!wxExAddressRange(ex, 5).Global(wxEmptyString, i));
    CPPUNIT_ASSERT(!wxExAddressRange(ex, 5).Global("XXX", i));
    CPPUNIT_ASSERT(!wxExAddressRange(ex, 5).Global("/", i));
    CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Global("/xx/p", i));
    CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Global("/xx/p#", i));
    CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Global("/xx/g", i));
    CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Global("/a/s/a/XX", i));
    CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Global("/b/s/b/XX|s/c/yy", i));
  }
  
  // Test Indent.
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent(false));
  
  // Test IsOk.
  // See above.
  
  // Test Join.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Join());
  CPPUNIT_ASSERT( stc->GetText().Contains("a"));
  CPPUNIT_ASSERT_MESSAGE( std::to_string(stc->GetLineCount()), stc->GetLineCount() == 1);
  
  // Test Move.
  stc->SetText(contents);
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "$")));
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  
  // Test Print.
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Print());
  
  // Test Sort.
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1").Sort("x"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("u"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("r"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("ur"));
  
  // Test Substitute.
  stc->SetText(contents);
  stc->GotoLine(1);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/tiger//"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("tiger"));
  
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/tiger/\\U&/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("TIGER"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("tiger"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\U"));
  
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("TIGER"));
  CPPUNIT_ASSERT( stc->GetText().Contains("tiger"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\U"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\L"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\0"));
  
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger/lion/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("lion"));
    
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("", '&'));
  CPPUNIT_ASSERT( stc->GetText().Contains("lion"));
  
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("", '~'));
  CPPUNIT_ASSERT( stc->GetText().Contains("lion"));
  
  stc->SetText("special char \\ present");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/\\\\//"));
  CPPUNIT_ASSERT_MESSAGE( stc->GetText().ToStdString(), stc->GetText().Contains("char  present"));
  
  stc->SetText("special char / present");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/\\///"));
  CPPUNIT_ASSERT( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char ' present");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/'///"));
  CPPUNIT_ASSERT( stc->GetText().Contains("char  present"));
  
  // Test Substitute and flags.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1").Substitute("//y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").Substitute("/x/y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/i"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/g"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("g", '&'));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("g", '~'));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Substitute("g", 'x'));

  // Test Write.
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Write("sample.txt"));
  CPPUNIT_ASSERT( remove("sample.txt") == 0);
  
  // Test Yank.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(0);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 2).Yank());
  stc->SelectNone();
  stc->AddText(stc->GetVi().GetMacros().GetRegister('0'));
  CPPUNIT_ASSERT( stc->GetLineCount() == 10);
  CPPUNIT_ASSERT( wxExAddressRange(ex, -2).Delete());
  stc->GotoLine(0);
  CPPUNIT_ASSERT( wxExAddressRange(ex, -2).Delete());
}
