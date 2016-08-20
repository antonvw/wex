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
    GetFrame(), "0123456789", wxExSTC::STC_WIN_HEX);
  AddPane(GetFrame(), stc);
    
  REQUIRE(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(48); // 0 <-
  
  wxExHexMode* hm = &stc->GetHexMode();
  
  REQUIRE( hm->Active());
  REQUIRE( hm->GetSTC() == stc);
  REQUIRE( hm->GetBuffer() == "0123456789");
  REQUIRE((hm->Printable('a') == 'a'));
  REQUIRE((hm->Printable(0) == '.'));
    
  hm->AppendText("0123456789");
  REQUIRE( hm->GetBuffer() == "01234567890123456789");
  REQUIRE( hm->HighlightOther()); // current pos
  REQUIRE( hm->HighlightOther(0));
  REQUIRE( hm->HighlightOther(10));
  REQUIRE( hm->HighlightOther(57));
  REQUIRE( hm->SetBuffer(0, ' '));
  REQUIRE( hm->GetBuffer() == " 1234567890123456789");
  hm->Undo();
  REQUIRE( hm->GetBuffer() == "01234567890123456789");
  
  wxExHexModeLine hex(hm);
  
  stc->DiscardEdits();  
  stc->Reload();
  REQUIRE(stc->GetText() == "01234567890123456789");
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 34 <- (ascii 4)
  REQUIRE( hex.IsHexField());
  REQUIRE(!hex.IsAsciiField());
  REQUIRE(!hex.IsReadOnly());
  REQUIRE(!hex.GetInfo().empty());
  REQUIRE(!hex.Replace('x'));
  REQUIRE(!hex.Replace('y'));
  REQUIRE(!hex.Replace('g'));
  REQUIRE( hex.Replace('a'));
  REQUIRE( hex.Replace('9'));
  REQUIRE( hex.Replace('2'));
  REQUIRE( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  INFO(wxExFileName(GetTestDir() + "test.hex").GetFullPath());
  REQUIRE( stc->GetFile().FileSave(wxExFileName(GetTestDir() + "test.hex")));
  stc->Reload();
  REQUIRE(stc->GetText() == "01232567890123456789");
  
  // Test ascii field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(54); // 6 <-
  REQUIRE(!hex.IsHexField());
  REQUIRE( hex.IsAsciiField());
  REQUIRE(!hex.IsReadOnly());
  REQUIRE(!hex.GetInfo().empty());
  REQUIRE( hex.Replace('x'));
  REQUIRE( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  REQUIRE( stc->GetFile().FileSave());
  stc->Reload();
  REQUIRE(stc->GetText() == "012325x7890123456789");
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(54); // valid
  REQUIRE( hex.Goto());
  hex.Set(9999); // invalid, should result in goto end
  REQUIRE( hex.Goto());
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 34 <- (ascii 4)
  REQUIRE( hex.ReplaceHex(32));
  hex.Set(55); // 7 <-
  REQUIRE(!hex.ReplaceHex(32));
  
  hm->Set(false);
  REQUIRE(!hm->Active());
  REQUIRE( hm->GetBuffer().empty());
  
  hm->AppendText("0123456789");
  REQUIRE(!hm->GetBuffer().empty());
  hm->Clear();
  REQUIRE( hm->GetBuffer().empty());

  hm->Set(false);
  REQUIRE(!hm->Active());
  hm->Set(true);
  REQUIRE( hm->Active());
  REQUIRE(!hm->SetBuffer(0, 30)); // should have no effect
  REQUIRE( hm->GetBuffer().empty());
  
  wxExLexers::Get()->Apply(stc);
  
  REQUIRE( remove("test.hex") == 0);
}
