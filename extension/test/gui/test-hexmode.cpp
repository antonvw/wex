////////////////////////////////////////////////////////////////////////////////
// Name:      test-hexmode.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/hexmode.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExHexMode", "[stc]")
{
  // 0000000000111111111122222222223333333333444444444455555555555666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 30 31 32 33 34 35 36 37 38 39                   0123456789
  wxExSTC* stc = new wxExSTC(
    GetFrame(), std::string("0123456789"), wxExSTC::STC_WIN_HEX);

  AddPane(GetFrame(), stc);
  REQUIRE(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(48); // 0 <-
  
  wxExHexMode* hm = &stc->GetHexMode();
  
  REQUIRE( hm->Active());
  REQUIRE( hm->GetSTC() == stc);
  REQUIRE( hm->GetBuffer() == "0123456789");
    
  hm->AppendText("0123456789");
  REQUIRE( hm->GetBuffer() == "01234567890123456789");
  REQUIRE( hm->HighlightOther()); // current pos
  REQUIRE( hm->HighlightOther(0));
  REQUIRE( hm->HighlightOther(10));
  REQUIRE( hm->HighlightOther(57));
  hm->Undo();
  REQUIRE( hm->GetBuffer() == "01234567890123456789");
  
  stc->DiscardEdits();  
  stc->Reload();
  REQUIRE( stc->GetText() == "01234567890123456789");
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  REQUIRE(!hm->GetInfo().empty()); // 34 <- (ascii 4)
  REQUIRE(!hm->Replace('x', 13));
  REQUIRE(!hm->Replace('y', 13));
  REQUIRE(!hm->Replace('g', 13));
  REQUIRE( hm->Replace('a', 13));
  REQUIRE( hm->Replace('9', 13));
  REQUIRE( hm->Replace('2', 13));
  
  INFO(wxExFileName(GetTestDir() + "test.hex").GetFullPath());
  REQUIRE( stc->GetFile().FileSave(wxExFileName(GetTestDir() + "test.hex")));
  stc->Reload();
  REQUIRE( stc->GetText() == "01232567890123456789");
  
  // Test ascii field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  REQUIRE(!hm->GetInfo().empty());
  REQUIRE( hm->Replace('x', 54)); // 6 <-
  
  REQUIRE( stc->GetFile().FileSave());
  stc->Reload();
  REQUIRE(stc->GetText() == "012325x7890123456789");
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  
  hm->Set(false);
  REQUIRE(!hm->Active());
  
  hm->AppendText("0123456789");
  REQUIRE(!hm->GetBuffer().empty());

  hm->Set(false);
  REQUIRE(!hm->Active());

  // Test delete, insert.
  stc->SetText("0123456789");
  hm->Set(true);
  REQUIRE( hm->Active());
  REQUIRE( hm->GetBuffer() == "0123456789");
  REQUIRE( hm->Delete(1, 13));
  REQUIRE( hm->GetBuffer() == "012356789");
  REQUIRE( hm->Insert("abc", 13));
  REQUIRE( hm->GetBuffer() == "0123abc56789");
  REQUIRE( hm->Insert("abc", 52)); // insert in ascii field 
  REQUIRE( hm->GetBuffer() == "0123abcabc56789");
  
  // Test replace target (replace in hex field).
  hm->SetText("0123456789");
  hm->Set(true);
  stc->SetTargetStart(wxSTC_INVALID_POSITION);
  REQUIRE(!hm->ReplaceTarget("AA"));
  stc->SetTargetStart(3);
  stc->SetTargetEnd(4);
  REQUIRE( hm->ReplaceTarget("AA"));
  stc->SetTargetStart(3); // second byte
  stc->SetTargetEnd(6);
  REQUIRE(!hm->ReplaceTarget("A"));
  REQUIRE(!hm->ReplaceTarget("AA AB"));
  REQUIRE(!hm->ReplaceTarget("FG"));
  REQUIRE(!hm->ReplaceTarget("aAAB"));
  REQUIRE( hm->ReplaceTarget("AAAB"));
  stc->SetTargetStart(3);
  stc->SetTargetEnd(6);
  REQUIRE( hm->ReplaceTarget("2021"));
  REQUIRE( hm->GetBuffer() == "0 !3456789");
  
  // If we have:
  // 30 31 32 33 34 35
  // RT: 31 32 -> 39
  //     30 39 33 34 35 (delete)
  // RT: 31 32 -> 39 39
  //     30 39 39 33 34 35 (replace)
  // RT: 31 32 -> 39 39 39
  //     30 39 39 39 33 34 35 (insert)
  hm->SetText("0123456789");
  REQUIRE( hm->GetBuffer() == "0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->ReplaceTarget("39"));
  REQUIRE( hm->GetBuffer() == "093456789");
  hm->SetText("0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->ReplaceTarget("3939"));
  REQUIRE( hm->GetBuffer() == "0993456789");
  hm->SetText("0123456789");
  stc->SetTargetStart(3);
  stc->SetTargetEnd(9);
  REQUIRE( hm->ReplaceTarget("393939"));
  REQUIRE( hm->GetBuffer() == "0999456789");

  // Test set text.
  hm->SetText("hello world");
  REQUIRE( hm->GetBuffer() == "hello world");
  REQUIRE( hm->GetSTC()->GetText() != "hello world");
  
  wxKeyEvent event(wxEVT_KEY_DOWN);
  hm->SetPos(event);
  
  wxExLexers::Get()->Apply(stc);
  
  REQUIRE( remove("test.hex") == 0);
}
