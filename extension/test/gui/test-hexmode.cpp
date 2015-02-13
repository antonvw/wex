////////////////////////////////////////////////////////////////////////////////
// Name:      test-hexmode.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/hexmode.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testHexMode()
{
  // 0000000000111111111122222222223333333333444444444455555555555666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 30 31 32 33 34 35 36 37 38 39                   0123456789
  wxExSTC* stc = new wxExSTC(
    m_Frame, "0123456789", wxExSTC::STC_WIN_HEX);
    
  CPPUNIT_ASSERT(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(48); // 0 <-
  
  wxExHexMode* hm = &stc->GetHexMode();
  
  CPPUNIT_ASSERT(hm->Active());
  CPPUNIT_ASSERT(hm->GetBuffer() == "0123456789");
    
  hm->AppendText("0123456789");
  CPPUNIT_ASSERT( hm->GetBuffer() == "01234567890123456789");
  CPPUNIT_ASSERT( hm->HighlightOther()); // current pos
  CPPUNIT_ASSERT( hm->HighlightOther(0));
  CPPUNIT_ASSERT( hm->HighlightOther(10));
  CPPUNIT_ASSERT( hm->HighlightOther(57));

  wxExHexModeLine hex(hm);
  
  stc->DiscardEdits();  
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "01234567890123456789");
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 34 <- (ascii 4)
  CPPUNIT_ASSERT( hex.IsHexField());
  CPPUNIT_ASSERT(!hex.IsAsciiField());
  CPPUNIT_ASSERT(!hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT(!hex.Replace('x'));
  CPPUNIT_ASSERT(!hex.Replace('y'));
  CPPUNIT_ASSERT(!hex.Replace('g'));
  CPPUNIT_ASSERT( hex.Replace('a'));
  CPPUNIT_ASSERT( hex.Replace('9'));
  CPPUNIT_ASSERT( hex.Replace('2'));
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->GetFile().FileSave(wxExFileName("test.hex"));
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "01232567890123456789");
  
  // Test ascii field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(54); // 6 <-
  CPPUNIT_ASSERT(!hex.IsHexField());
  CPPUNIT_ASSERT( hex.IsAsciiField());
  CPPUNIT_ASSERT(!hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT( hex.Replace('x'));
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->GetFile().FileSave();
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "012325x7890123456789");
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(54); // valid
  CPPUNIT_ASSERT( hex.Goto());
  hex.Set(9999); // invalid, should result in goto end
  CPPUNIT_ASSERT( hex.Goto());
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 34 <- (ascii 4)
  CPPUNIT_ASSERT( hex.ReplaceHex(32));
  hex.Set(55); // 7 <-
  CPPUNIT_ASSERT(!hex.ReplaceHex(32));
  
  hm->Set(false);
  CPPUNIT_ASSERT(!hm->Active());
  CPPUNIT_ASSERT( hm->GetBuffer().empty());
}
