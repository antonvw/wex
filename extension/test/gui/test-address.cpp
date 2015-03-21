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
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void wxExGuiTestFixture::testAddress()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  const int lines = stc->GetLineCount();
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLineAndSelect(1);
  ex->MarkerAdd('a'); // put marker a on line
  stc->GotoLineAndSelect(2);
  ex->MarkerAdd('b'); // put marker b on line
  
  CPPUNIT_ASSERT( wxExAddress(ex).GetLine() == 0);
  
  for (const auto& it : std::vector< std::pair<std::string, int>> {
    {"30", lines},
    {"40", lines},
    {"-40", 1},
    {"3-3", 0},
    {"3-1", 2},
    {".", 2},
    {".+1", 3},
    {"$", lines},
    {"$-2", lines - 2},
    {"x", 0},
    {"'x", 0},
    {"1,3s/x/y", 0},
    {"'a", 1},
    {"'b", 2},
    {"'b+10", lines},
    {"10+'b", lines},
    {"'a+'b", 3},
    {"'b+'a", 3},
    {"'b-'a", 1}})
  {
    CPPUNIT_ASSERT( wxExAddress(ex, it.first).GetLine() == it.second);
  }

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
  
  // Test Sort and flags.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1").Sort("x"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("u"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("r"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1").Sort("ur"));
  
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
  const wxString contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  
  stc->SetText(contents);
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Filter("uniq"));
  CPPUNIT_ASSERT( stc->GetLineCount() == 5);
  
  // Test Move.
  stc->SetText(contents);
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "$")));
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  
  // Test Indent.
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Indent(false));
  
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
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/tiger/lion/"));
    
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/tiger/~/"));
  CPPUNIT_ASSERT( stc->GetText().Contains("lion"));
  
  stc->SetText("special char \\ present");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/\\\\//"));
  CPPUNIT_ASSERT( stc->GetText().Contains("char  present"));
  
  stc->SetText("special char / present");
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").
    Substitute("/\\///"));
  CPPUNIT_ASSERT( stc->GetText().Contains("char  present"));
  
  // Test Write.
  stc->SetText(contents);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Write("sample.txt"));
  CPPUNIT_ASSERT( remove("sample.txt") == 0);
}
