////////////////////////////////////////////////////////////////////////////////
// Name:      test-frd.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testFrd()
{
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  CPPUNIT_ASSERT(frd != nullptr);
  
  wxExFindTextCtrl* tc = new wxExFindTextCtrl(m_Frame);
  AddPane(m_Frame, tc);
  
  frd->SetMatchCase(true);
  CPPUNIT_ASSERT( frd->MatchCase());
  frd->SetMatchWord(true);
  CPPUNIT_ASSERT( frd->MatchWord());
  frd->SetUseRegEx(true);
  CPPUNIT_ASSERT( frd->UseRegEx());

  frd->SetFindString("find1");
  frd->SetFindString("find2");
  frd->SetFindString("find[0-9]");
  CPPUNIT_ASSERT( frd->UseRegEx());
  CPPUNIT_ASSERT(!frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetFindString() == "find[0-9]");
  
  CPPUNIT_ASSERT( frd->RegExMatches("some text find9 other text"));
  CPPUNIT_ASSERT(!frd->RegExMatches("some text finda other text"));
  
  CPPUNIT_ASSERT( frd->Iterate(tc, WXK_UP));
  CPPUNIT_ASSERT( frd->Iterate(tc, WXK_UP));
  CPPUNIT_ASSERT( frd->Iterate(tc, WXK_DOWN));
  CPPUNIT_ASSERT(!frd->Iterate(tc, WXK_RIGHT));

  const std::list < wxString > l{"find3","find4","find5"};
  frd->SetFindStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  
  frd->SetReplaceString("replace1");
  frd->SetReplaceString("replace2");
  frd->SetReplaceString("replace[0-9]");
  CPPUNIT_ASSERT(!frd->GetReplaceStrings().empty());
  CPPUNIT_ASSERT( frd->GetReplaceString() == "replace[0-9]");
  
  CPPUNIT_ASSERT(!frd->GetTextFindWhat().empty());
  CPPUNIT_ASSERT(!frd->GetTextMatchCase().empty());
  CPPUNIT_ASSERT(!frd->GetTextMatchWholeWord().empty());
  CPPUNIT_ASSERT(!frd->GetTextRegEx().empty());
  CPPUNIT_ASSERT(!frd->GetTextReplaceWith().empty());
  CPPUNIT_ASSERT(!frd->GetTextSearchDown().empty());
  
  frd->SetReplaceStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  CPPUNIT_ASSERT( frd->GetReplaceString() == "find3");
  CPPUNIT_ASSERT( frd->SearchDown());
  
  const std::list< wxString > e;
  frd->SetFindStrings(e);
  frd->SetReplaceStrings(e);
  CPPUNIT_ASSERT( frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetFindString().empty());
  CPPUNIT_ASSERT( frd->GetReplaceStrings().empty());
  CPPUNIT_ASSERT( frd->GetReplaceString().empty());
  
  frd->SetFindString("find[0-9]");
  frd->SetReplaceString("xxx");
  std::string text("find1 find2 find3 find4");
  const int res = frd->RegExReplaceAll(text);
  CPPUNIT_ASSERT_MESSAGE( std::to_string(res), res == 4);
  
  frd->SetFindString("find[0-9");
  CPPUNIT_ASSERT(!frd->UseRegEx());
  frd->SetUseRegEx(true);
  CPPUNIT_ASSERT(!frd->UseRegEx());
  // take care we end with valid regex
  frd->SetFindString("find[0-9]");
  frd->SetUseRegEx(true);
  CPPUNIT_ASSERT( frd->UseRegEx());
}
