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

void wxExGuiTestFixture::testFrd()
{
  CPPUNIT_ASSERT(wxExFindReplaceData::Get() != NULL);
  
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  wxTextCtrl* tc = new wxTextCtrl(m_Frame, wxID_ANY, "hello text");
  
  frd->SetMatchCase(true);
  CPPUNIT_ASSERT( frd->MatchCase());
  frd->SetMatchWord(true);
  CPPUNIT_ASSERT( frd->MatchWord());
  frd->SetUseRegularExpression(true);
  CPPUNIT_ASSERT( frd->UseRegularExpression());

  frd->SetFindString("find1");
  frd->SetFindString("find2");
  frd->SetFindString("find[0-9]");
  CPPUNIT_ASSERT(!frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetFindString() == "find[0-9]");
  CPPUNIT_ASSERT( frd->GetRegularExpression().IsValid());
  CPPUNIT_ASSERT( frd->GetRegularExpression().Matches("find9"));
  
  CPPUNIT_ASSERT( frd->Iterate(tc, WXK_UP));
  CPPUNIT_ASSERT( frd->Iterate(tc, WXK_UP));
  CPPUNIT_ASSERT(!frd->Iterate(tc, WXK_RIGHT));

  std::list < wxString > l;
  l.push_back("find3");
  l.push_back("find4");
  l.push_back("find5");
  frd->SetFindStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  
  frd->SetReplaceString("replace1");
  frd->SetReplaceString("replace2");
  frd->SetReplaceString("replace[0-9]");
  CPPUNIT_ASSERT(!frd->GetReplaceStrings().empty());
  CPPUNIT_ASSERT( frd->GetReplaceString() == "replace[0-9]");
  
  frd->SetReplaceStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  CPPUNIT_ASSERT( frd->GetReplaceString() == "find3");
  
  CPPUNIT_ASSERT( frd->Set(frd->GetTextMatchCase(), false));
  CPPUNIT_ASSERT(!frd->MatchCase());
  CPPUNIT_ASSERT( frd->Set(frd->GetTextMatchWholeWord(), false));
  CPPUNIT_ASSERT(!frd->MatchWord());
  CPPUNIT_ASSERT(!frd->Set("XXXX", false));
  
  const std::list< wxString > e;
  frd->SetFindStrings(e);
  CPPUNIT_ASSERT( frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetFindString().empty());
  
  frd->SetReplaceStrings(e);
  CPPUNIT_ASSERT( frd->GetReplaceStrings().empty());
  CPPUNIT_ASSERT( frd->GetReplaceString().empty());
}
