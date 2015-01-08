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
#include <wx/extension/address.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void wxExGuiTestFixture::testAddress()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  const int lines = stc->GetLineCount();
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLineAndSelect(1);
  ex->MarkerAdd('a'); // put marker a on line
  stc->GotoLineAndSelect(2);
  ex->MarkerAdd('b'); // put marker b on line
  
  CPPUNIT_ASSERT( wxExAddress(ex).GetLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "30").GetLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "40").GetLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "-40").GetLine() == 1);
  CPPUNIT_ASSERT( wxExAddress(ex, "3-3").GetLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "3-1").GetLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, ".").GetLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, ".+1").GetLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "$").GetLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "$-2").GetLine() == lines - 2);
  CPPUNIT_ASSERT( wxExAddress(ex, "x").GetLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "'x").GetLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "1,3s/x/y").GetLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "'a").GetLine() == 1);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b").GetLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b+10").GetLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "10+'b").GetLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "'a+'b").GetLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b+'a").GetLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b-'a").GetLine() == 1);
  
  wxExAddress address(ex);
  CPPUNIT_ASSERT( address.GetLine() == 0);
  address.SetLine(-1);
  CPPUNIT_ASSERT( address.GetLine() == 1);
  address.SetLine(1);
  CPPUNIT_ASSERT( address.GetLine() == 1);
  address.SetLine(100);
  CPPUNIT_ASSERT( address.GetLine() == lines);
  
  wxExAddress address2(ex, "'a");
  CPPUNIT_ASSERT( address2.GetLine() == 1);
  address2.MarkerDelete();
  CPPUNIT_ASSERT( address2.GetLine() == 0);
}

void wxExGuiTestFixture::testAddressRange()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello\nhello1\nhello2");
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLine(2);

  // Test valid ranges when no selection is active.
  CPPUNIT_ASSERT( wxExAddressRange(ex).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, -1).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "*").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, ".").IsOk());
  
  // Test invalid ranges when no selection is active.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").Delete());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Filter("ls"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Indent());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,!").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,@").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "x")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,3").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Write("flut"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, " ,").Yank());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "'<,'>").IsOk());
  
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
  
  // Test Substitute and flags.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1").Substitute("//y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").Substitute("/x/y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/i"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/g"));

  // Test implementation.  
  
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
  
  // Test Filter.
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Filter("uniq"));
  CPPUNIT_ASSERT( stc->GetLineCount() == 5);
  
  // Test Move.
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "$")));
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  
  // Test Indent.
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent(false));
  
  // Test Substitute.
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  stc->GotoLine(1);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger//"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("tiger"));
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger/\\U&/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("TIGER"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("tiger"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\U"));
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("TIGER"));
  CPPUNIT_ASSERT( stc->GetText().Contains("tiger"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\U"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\L"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("\\0"));
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger/lion/"));
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Substitute("/tiger/~/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("lion"));
  
  // Test Write.
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Write("sample.txt"));
  CPPUNIT_ASSERT( remove("sample.txt") == 0);
}
