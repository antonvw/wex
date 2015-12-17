////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/filehistory.h>
#include <wx/numformatter.h>
#include <wx/extension/util.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void fixture::testToVectorString()
{
  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
  wxExToVectorString v1(a);
  CPPUNIT_ASSERT( v1.Get().size() == 4);
  
  wxExToVectorString v3("test test test");
  CPPUNIT_ASSERT( v3.Get().size() == 3);
}

void fixture::testUtil()
{
  wxExSTC* stc = new wxExSTC(m_Frame);
  AddPane(m_Frame, stc);
  stc->SetFocus();
  
  // wxExAlignText
  CPPUNIT_ASSERT( wxExAlignText("test", "header", true, true,
    wxExLexers::Get()->FindByName("cpp")).size() 
      == wxString("// headertest").size());
      
  // wxExAutoComplete
  CPPUNIT_ASSERT( wxExViMacros::LoadDocument());
  wxString s;
  CPPUNIT_ASSERT(!wxExAutoComplete("xxxx", 
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT(!wxExAutoComplete("Date", // not unique!
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT( wxExAutoComplete("Datet", 
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT( s == "Datetime");
  
  // wxExAutoCompleteFileName
  std::vector<wxString> v;
  CPPUNIT_ASSERT( wxExAutoCompleteFileName("te", v));
  
#ifdef DEBUG  
  for (int i = 0; i < v.size(); i++)
  {
    wxLogMessage("v[%d]=%s", i, v[i].c_str());
  }
#endif

  CPPUNIT_ASSERT( v[0] == "st");
  CPPUNIT_ASSERT(!wxExAutoCompleteFileName("XX", v));
  CPPUNIT_ASSERT( v[0] == "st");
  
  CPPUNIT_ASSERT( wxExAutoCompleteFileName("/usr/include/s", v));
  CPPUNIT_ASSERT( wxExAutoCompleteFileName("../../../extension/src/v", v));
  CPPUNIT_ASSERT( wxExAutoCompleteFileName("~/", v));
  
  // wxExCalculator
  stc->SetText("aaaaa\nbbbbb\nccccc\n");
  const wxChar ds(wxNumberFormatter::GetDecimalSeparator());
  wxExEx* ex = new wxExEx(stc);
  int width = 0;
  
  std::vector<std::pair<std::string, std::pair<double, int>>>calcs{
    {"",       {0,0}},
    {"  ",     {0,0}},
    {"1 + 1",  {2,0}},
    {"5+5",    {10,0}},
    {"1 * 1",  {1,0}},
    {"1 - 1",  {0,0}},
    {"2 / 1",  {2,0}},
    {"2 / 0",  {0,0}},
    {"2 << 2", {8,0}},
    {"2 >> 1", {1,0}},
    {"2 | 1",  {3,0}},
    {"2 & 1",  {0,0}},
    {"4 % 3",  {1,0}},
    {".",      {1,0}},
    {"xxx",    {0,0}},
    {"%s",     {0,0}},
    {"%/xx/",  {0,0}},
    {"$",      {4,0}}};
    
  if (ds == '.')
  {
    calcs.insert(calcs.end(), {{"1.0 + 1",{2,1}},{"1.1 + 1.1",{2.2,1}}});
  }
  else
  {
    calcs.insert(calcs.end(), {{"1,0 + 1",{2,1}},{"1,1 + 1,1",{2.2,1}}});
  }
    
  for (const auto& calc : calcs)
  {
    CPPUNIT_ASSERT( wxExCalculator(calc.first, ex, width) == calc.second.first);
    CPPUNIT_ASSERT( width == calc.second.second);
  }

  // wxExClipboardAdd
  CPPUNIT_ASSERT( wxExClipboardAdd("test"));
  
  // wxExClipboardGet
  CPPUNIT_ASSERT( wxExClipboardGet() == "test");
  
  // wxExComboBoxFromList
  std::list < wxString > l{"x","y","z"};
  wxComboBox* cb = new wxComboBox(m_Frame, wxID_ANY);
  AddPane(m_Frame, cb);
  wxExComboBoxFromList(cb, l);
  CPPUNIT_ASSERT( cb->GetCount() == 3);
  
  // wxExComboBoxToList
  l.clear();
  l = wxExComboBoxToList(cb);
  CPPUNIT_ASSERT( l.size() == 3);
  
  // wxExCompareFile
  
  // wxExConfigFirstOf
  wxExConfigFirstOf("xxxx");
  
  // wxExConfigFirstOfWrite
  CPPUNIT_ASSERT( wxExConfigFirstOfWrite("xxxx","zz") == "zz");
  
  // wxExEllipsed  
  CPPUNIT_ASSERT( wxExEllipsed("xxx").Contains("..."));
  
  // wxExGetEndOfText
  CPPUNIT_ASSERT( wxExGetEndOfText("test", 3).size() == 3);
  CPPUNIT_ASSERT( wxExGetEndOfText("testtest", 3).size() == 3);
  
  // wxExGetFieldSeparator
  CPPUNIT_ASSERT( wxExGetFieldSeparator() != 'a');

  // wxExGetFindResult  
  CPPUNIT_ASSERT( wxExGetFindResult("test", true, true).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", true, false).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", false, true).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", false, false).Contains("test"));
  
  CPPUNIT_ASSERT( wxExGetFindResult("%d", true, true).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", true, false).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", false, true).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", false, false).Contains("%d"));
  
  // wxExGetIconID
  CPPUNIT_ASSERT( wxExGetIconID( GetTestFile()) != -1);

  // wxExGetNumberOfLines  
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test") == 1);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\n") == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\ntest") == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\rtest\r") == 3);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n") == 3);
  
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n\n", true) == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
  
  // wxExGetWord
  
  // wxExIsBrace
  std::vector<int> cs{'(',')','{','<','>'};
  
  for (const auto& c : cs)
  {
    CPPUNIT_ASSERT( wxExIsBrace(c));
  }

  CPPUNIT_ASSERT(!wxExIsBrace('a'));
  
  // wxExIsCodewordSeparator
  cs.insert(cs.end(), {',',';',':','@'});
  
  for (const auto& c : cs)
  {
    CPPUNIT_ASSERT( wxExIsCodewordSeparator(c));
  }
  
  CPPUNIT_ASSERT(!wxExIsCodewordSeparator('x'));

  // wxExListFromConfig
  CPPUNIT_ASSERT( wxExListFromConfig("xxx").size() == 0);
  
  // wxExListToConfig
  l.clear();
  l.push_back("1");
  l.push_back("2");
  wxExListToConfig(l, "list_items");
  CPPUNIT_ASSERT( l.size() == 2);
  CPPUNIT_ASSERT(wxConfigBase::Get()->Read("list_items", "").Contains("1"));
  CPPUNIT_ASSERT(wxConfigBase::Get()->Read("list_items", "").Contains("2"));
  
  // wxExLogStatus
  wxExLogStatus( GetTestFile());

  // wxExMake  
  const wxString wd = wxGetCwd(); // as /usr/bin/git changes wd
  CPPUNIT_ASSERT( wxExMake(wxFileName("xxx")) != -1);
  CPPUNIT_ASSERT( wxExMake(wxFileName("make.tst")) != -1);
  CPPUNIT_ASSERT( wxExMake(wxFileName("/usr/bin/git")) != -1);
  wxSetWorkingDirectory(wd);
  
  // wxExMatch
  CPPUNIT_ASSERT( wxExMatch("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
  CPPUNIT_ASSERT( wxExMatch("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
  CPPUNIT_ASSERT( wxExMatch(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
  CPPUNIT_ASSERT( wxExMatch("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  
  // wxExMatchesOneOf
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.txt"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  
  // wxExNodeProperties
  
  // wxExNodeStyles
  
  // wxExOpenFiles
  CPPUNIT_ASSERT(!wxExOpenFiles(m_Frame, std::vector<wxString>()));
  CPPUNIT_ASSERT(!wxExOpenFiles(m_Frame, std::vector<wxString> {
    GetTestFile().GetFullPath(), "test.cpp", "*xxxxxx*.cpp"}));
  CPPUNIT_ASSERT_MESSAGE( GetTestFile().GetFullPath().ToStdString(), wxExOpenFiles(m_Frame, std::vector<wxString> {GetTestFile().GetFullPath()}));
  CPPUNIT_ASSERT( wxExOpenFiles(m_Frame, std::vector<wxString> {"../../data/vcs.xml"}));

  // wxExOpenFilesDialog
  
  // wxExPrintCaption
  CPPUNIT_ASSERT( wxExPrintCaption(wxFileName("test")).Contains("test"));
  
  // wxExPrintFooter
  CPPUNIT_ASSERT( wxExPrintFooter().Contains("@"));
  
  // wxExPrintHeader
  CPPUNIT_ASSERT( wxExPrintHeader(GetTestFile()).Contains("test"));
  
  // wxExQuoted
  CPPUNIT_ASSERT( wxExQuoted("test") == "'test'");
  CPPUNIT_ASSERT( wxExQuoted("%d") == "'%d'");
  CPPUNIT_ASSERT( wxExQuoted(wxExSkipWhiteSpace(wxString(" %d "))) == "'%d'");
  
  // wxExReplaceMarkers.
  stc->SetText("aaaaa\nbbbbb\nccccc\n");
  CPPUNIT_ASSERT(ex->MarkerAdd('a', 1));
  CPPUNIT_ASSERT(ex->MarkerAdd('t', 1));
  CPPUNIT_ASSERT(ex->MarkerAdd('u', 2));
  wxString text;
  CPPUNIT_ASSERT( wxExReplaceMarkers(text, ex));
  text = "'a";
  CPPUNIT_ASSERT( wxExReplaceMarkers(text, ex));
  CPPUNIT_ASSERT( text == "2");
  text = "'t,'u,therest";
  CPPUNIT_ASSERT( wxExReplaceMarkers(text, ex));
  CPPUNIT_ASSERT( text == "2,3,therest");
  text = "'z";
  CPPUNIT_ASSERT(!wxExReplaceMarkers(text, ex));

  // wxExSetTextCtrlValue
  
  // wxExSort
  CPPUNIT_ASSERT(wxExSort("z\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\n");
  CPPUNIT_ASSERT(wxExSort("z\ny\nx\n", STRING_SORT_DESCENDING, 0, "\n") == "z\ny\nx\n");
  CPPUNIT_ASSERT(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\nz\n");
  CPPUNIT_ASSERT(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING | STRING_SORT_UNIQUE, 0, "\n") == "x\ny\nz\n");
  
  const wxString rect("\
012z45678901234567890\n\
123y56789012345678901\n\
234x67890123456789012\n\
345a78901234567890123\n\
456b89012345678901234\n");

  const wxString sorted("\
012a78908901234567890\n\
123b89019012345678901\n\
234x67890123456789012\n\
345y56781234567890123\n\
456z45672345678901234\n");

  CPPUNIT_ASSERT(wxExSort(rect, STRING_SORT_ASCENDING, 3, "\n", 5) == sorted);

  // wxExSortSelection
  stc->SelectNone();
  CPPUNIT_ASSERT( wxExSortSelection(stc));
  stc->SetText("aaaaa\nbbbbb\nccccc\n");
  stc->SelectAll();
  CPPUNIT_ASSERT( wxExSortSelection(stc));
  CPPUNIT_ASSERT( wxExSortSelection(stc, STRING_SORT_ASCENDING, 3, 10));
  CPPUNIT_ASSERT(!wxExSortSelection(stc, STRING_SORT_ASCENDING, 20, 10));
  stc->SelectNone();
  stc->SetText(rect);
  // force rectangular selection.
  (void)stc->GetVi().Command("3 ");
  (void)stc->GetVi().Command("F");
  (void)stc->GetVi().Command("4j");
  (void)stc->GetVi().Command("5l");
  CPPUNIT_ASSERT( wxExSortSelection(stc, STRING_SORT_ASCENDING, 3, 5));
  CPPUNIT_ASSERT( stc->GetText() == sorted);
  CPPUNIT_ASSERT( wxExSortSelection(stc, STRING_SORT_DESCENDING, 3, 5));
  CPPUNIT_ASSERT( stc->GetText() != sorted);
  
  // wxExSkipWhiteSpace
  CPPUNIT_ASSERT( wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  
  // wxExTranslate
  CPPUNIT_ASSERT(!wxExTranslate(
    "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
    
  // wxExVCSCommandOnSTC
  wxExVCSCommand command("status");
  wxExVCSCommandOnSTC(command, wxExLexer("cpp"), stc);
  
  // wxExVCSExecute
  // wxExVCSExecute(m_Frame, 0, files); // calls dialog
  
  delete ex;
}
