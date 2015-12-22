////////////////////////////////////////////////////////////////////////////////
// Name:      test-frd.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExFrd")
{
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  REQUIRE(frd != nullptr);
  
  wxExFindTextCtrl* tc = new wxExFindTextCtrl(GetFrame());
  AddPane(GetFrame(), tc);
  
  frd->SetMatchCase(true);
  REQUIRE( frd->MatchCase());
  frd->SetMatchWord(true);
  REQUIRE( frd->MatchWord());
  frd->SetUseRegEx(true);
  REQUIRE( frd->UseRegEx());

  frd->SetFindString("find1");
  frd->SetFindString("find2");
  frd->SetFindString("find[0-9]");
  REQUIRE( frd->UseRegEx());
  REQUIRE(!frd->GetFindStrings().empty());
  REQUIRE( frd->GetFindString() == "find[0-9]");
  
  REQUIRE( frd->RegExMatches("some text find9 other text"));
  REQUIRE(!frd->RegExMatches("some text finda other text"));
  
  REQUIRE( frd->Iterate(tc, WXK_UP));
  REQUIRE( frd->Iterate(tc, WXK_UP));
  REQUIRE( frd->Iterate(tc, WXK_DOWN));
  REQUIRE(!frd->Iterate(tc, WXK_RIGHT));

  const std::list < wxString > l{"find3","find4","find5"};
  frd->SetFindStrings(l);
  REQUIRE( frd->GetFindString() == "find3");
  
  frd->SetReplaceString("replace1");
  frd->SetReplaceString("replace2");
  frd->SetReplaceString("replace[0-9]");
  REQUIRE(!frd->GetReplaceStrings().empty());
  REQUIRE( frd->GetReplaceString() == "replace[0-9]");
  
  REQUIRE(!frd->GetTextFindWhat().empty());
  REQUIRE(!frd->GetTextMatchCase().empty());
  REQUIRE(!frd->GetTextMatchWholeWord().empty());
  REQUIRE(!frd->GetTextRegEx().empty());
  REQUIRE(!frd->GetTextReplaceWith().empty());
  REQUIRE(!frd->GetTextSearchDown().empty());
  
  frd->SetReplaceStrings(l);
  REQUIRE( frd->GetFindString() == "find3");
  REQUIRE( frd->GetReplaceString() == "find3");
  REQUIRE( frd->SearchDown());
  
  const std::list< wxString > e;
  frd->SetFindStrings(e);
  frd->SetReplaceStrings(e);
  REQUIRE( frd->GetFindStrings().empty());
  REQUIRE( frd->GetFindString().empty());
  REQUIRE( frd->GetReplaceStrings().empty());
  REQUIRE( frd->GetReplaceString().empty());
  
  frd->SetFindString("find[0-9]");
  frd->SetReplaceString("xxx");
  std::string text("find1 find2 find3 find4");
  const int res = frd->RegExReplaceAll(text);
  INFO( std::to_string(res));
  REQUIRE( res == 4);
  
  frd->SetFindString("find[0-9");
  REQUIRE(!frd->UseRegEx());
  frd->SetUseRegEx(true);
  REQUIRE(!frd->UseRegEx());
  // take care we end with valid regex
  frd->SetFindString("find[0-9]");
  frd->SetUseRegEx(true);
  REQUIRE( frd->UseRegEx());
}
