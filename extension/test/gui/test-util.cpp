////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExToVectorString")
{
  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
  wxExToVectorString v1(a);
  REQUIRE( v1.Get().size() == 4);
  
  wxExToVectorString v3("test test test");
  REQUIRE( v3.Get().size() == 3);
}

TEST_CASE("wxEx", "[stc][vi]")
{
  std::list < wxString > l{"x","y","z"};
  std::vector<int> cs{'(',')','{','<','>'};
    
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

  SECTION("wxExAlignText")
  {
    REQUIRE( wxExAlignText("test", "header", true, true,
      wxExLexers::Get()->FindByName("cpp")).size() 
        == wxString("// headertest").size());
  }
      
  SECTION("wxExAutoComplete")
  {
    REQUIRE( wxExViMacros::LoadDocument());
    wxString s;
    REQUIRE(!wxExAutoComplete("xxxx", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE(!wxExAutoComplete("Date", // not unique!
      GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( wxExAutoComplete("Datet", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( s == "Datetime");
  }
  
  SECTION("wxExAutoCompleteFileName")
  {
    std::vector<wxString> v;
    REQUIRE( wxExAutoCompleteFileName("te", v));
    
#ifdef DEBUG  
    for (int i = 0; i < v.size(); i++)
    {
      wxLogMessage("v[%d]=%s", i, v[i].c_str());
    }
#endif

    REQUIRE( v[0] == "st");
    REQUIRE(!wxExAutoCompleteFileName("XX", v));
    REQUIRE( v[0] == "st");
    
    REQUIRE( wxExAutoCompleteFileName("/usr/include/s", v));
    REQUIRE( wxExAutoCompleteFileName("../../../extension/src/v", v));
    REQUIRE( wxExAutoCompleteFileName("~/", v));
  }
  
  SECTION("wxExCalculator")
  {
    GetSTC()->SetText("aaaaa\nbbbbb\nccccc\n");
    const wxChar ds(wxNumberFormatter::GetDecimalSeparator());
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
      
    wxExEx* ex = new wxExEx(GetSTC());
  
    for (const auto& calc : calcs)
    {
      REQUIRE( wxExCalculator(calc.first, ex, width) == calc.second.first);
      REQUIRE( width == calc.second.second);
    }
    
    delete ex;
  }

  SECTION("wxExClipboardAdd")
  {
    REQUIRE( wxExClipboardAdd("test"));
  }
  
  SECTION("wxExClipboardGet")
  {
    REQUIRE( wxExClipboardGet() == "test");
  }
  
  SECTION("wxExComboBoxFromList")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
    AddPane(GetFrame(), cb);
    wxExComboBoxFromList(cb, l);
    REQUIRE( cb->GetCount() == 3);
  }
  
  SECTION("wxExComboBoxToList")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
    AddPane(GetFrame(), cb);
    wxExComboBoxFromList(cb, l);
    l = wxExComboBoxToList(cb);
    REQUIRE( l.size() == 3);
  }
  
  SECTION("wxExCompareFile")
  {
  }
  
  SECTION("wxExConfigFirstOf")
  {
    wxExConfigFirstOf("xxxx");
  }
  
  SECTION("wxExConfigFirstOfWrite")
  {
    REQUIRE( wxExConfigFirstOfWrite("xxxx","zz") == "zz");
  }
  
  SECTION("wxExEllipsed  ")
  {
    REQUIRE( wxExEllipsed("xxx").Contains("..."));
  }
  
  SECTION("wxExGetEndOfText")
  {
    REQUIRE( wxExGetEndOfText("test", 3).size() == 3);
    REQUIRE( wxExGetEndOfText("testtest", 3).size() == 3);
  }
  
  SECTION("wxExGetFieldSeparator")
  {
    REQUIRE((wxExGetFieldSeparator() != 'a'));
  }

  SECTION("wxExGetFindResult")
  {
    REQUIRE( wxExGetFindResult("test", true, true).Contains("test"));
    REQUIRE( wxExGetFindResult("test", true, false).Contains("test"));
    REQUIRE( wxExGetFindResult("test", false, true).Contains("test"));
    REQUIRE( wxExGetFindResult("test", false, false).Contains("test"));
    
    REQUIRE( wxExGetFindResult("%d", true, true).Contains("%d"));
    REQUIRE( wxExGetFindResult("%d", true, false).Contains("%d"));
    REQUIRE( wxExGetFindResult("%d", false, true).Contains("%d"));
    REQUIRE( wxExGetFindResult("%d", false, false).Contains("%d"));
  }
  
  SECTION("wxExGetIconID")
  {
    REQUIRE( wxExGetIconID( GetTestFile()) != -1);
  }

  SECTION("wxExGetNumberOfLines  ")
  {
    REQUIRE( wxExGetNumberOfLines("test") == 1);
    REQUIRE( wxExGetNumberOfLines("test\n") == 2);
    REQUIRE( wxExGetNumberOfLines("test\ntest") == 2);
    REQUIRE( wxExGetNumberOfLines("test\ntest\n") == 3);
    REQUIRE( wxExGetNumberOfLines("test\rtest\r") == 3);
    REQUIRE( wxExGetNumberOfLines("test\r\ntest\n") == 3);
    
    REQUIRE( wxExGetNumberOfLines("test\r\ntest\n\n\n", true) == 2);
    REQUIRE( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
    REQUIRE( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
  }
  
  SECTION("wxExGetWord")
  {
  }
  
  SECTION("wxExIsBrace")
  {
    for (const auto& c : cs)
    {
      REQUIRE( wxExIsBrace(c));
    }
    
    REQUIRE(!wxExIsBrace('a'));
  }

  SECTION("wxExIsCodewordSeparator")
  {
    cs.insert(cs.end(), {',',';',':','@'});
    
    for (const auto& c : cs)
    {
      REQUIRE( wxExIsCodewordSeparator(c));
    }
    
    REQUIRE(!wxExIsCodewordSeparator('x'));
  }
  
  SECTION("wxExListFromConfig")
  {
    REQUIRE( wxExListFromConfig("xxx").size() == 0);
  }
  
  SECTION("wxExListToConfig")
  {
    l.clear();
    l.push_back("1");
    l.push_back("2");
    wxExListToConfig(l, "list_items");
    REQUIRE( l.size() == 2);
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("1"));
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("2"));
  }
  
  SECTION("wxExLogStatus")
  {
    wxExLogStatus( GetTestFile());
  }

  SECTION("wxExMake")
  {
    const wxString wd = wxGetCwd(); // as /usr/bin/git changes wd
    REQUIRE( wxExMake(wxFileName("xxx")) != -1);
    REQUIRE( wxExMake(wxFileName("make.tst")) != -1);
    REQUIRE( wxExMake(wxFileName("/usr/bin/git")) != -1);
    wxSetWorkingDirectory(wd);
  }
  
  SECTION("wxExMatch")
  {
    std::vector<wxString> v;
    REQUIRE( wxExMatch("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wxExMatch("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wxExMatch(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE( wxExMatch("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }
  
  SECTION("wxExMatchesOneOf")
  {
    REQUIRE(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
    REQUIRE( wxExMatchesOneOf(wxFileName("test.txt"), "*.txt"));
    REQUIRE( wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  }
  
  SECTION("wxExNodeProperties")
  {
  }
  
  SECTION("wxExNodeStyles")
  {
  }
  
  SECTION("wxExOpenFiles")
  {
    REQUIRE( wxExOpenFiles(GetFrame(), std::vector<wxString>()) == 0);
    REQUIRE( wxExOpenFiles(GetFrame(), std::vector<wxString> {
      GetTestFile().GetFullPath(), "test.cpp", "*xxxxxx*.cpp"}) == 2);
    INFO( GetTestFile().GetFullPath().ToStdString()); 
    REQUIRE( wxExOpenFiles(GetFrame(), 
        std::vector<wxString> {GetTestFile().GetFullPath()}) == 1);
    REQUIRE( 
      wxExOpenFiles(GetFrame(), std::vector<wxString> {"../../data/vcs.xml"}) == 1);
  }

  SECTION("wxExOpenFilesDialog")
  {
  }
  
  SECTION("wxExPrintCaption")
  {
    REQUIRE( wxExPrintCaption(wxFileName("test")).Contains("test"));
  }
  
  SECTION("wxExPrintFooter")
  {
    REQUIRE( wxExPrintFooter().Contains("@"));
  }
  
  SECTION("wxExPrintHeader")
  {
    REQUIRE( wxExPrintHeader(GetTestFile()).Contains("test"));
  }
  
  SECTION("wxExQuoted")
  {
    REQUIRE( wxExQuoted("test") == "'test'");
    REQUIRE( wxExQuoted("%d") == "'%d'");
    REQUIRE( wxExQuoted(wxExSkipWhiteSpace(wxString(" %d "))) == "'%d'");
  }
  
  SECTION("wxExReplaceMarkers.")
  {
    wxExEx* ex = new wxExEx(GetSTC());
  
    GetSTC()->SetText("aaaaa\nbbbbb\nccccc\n");
    REQUIRE(ex->MarkerAdd('a', 1));
    REQUIRE(ex->MarkerAdd('t', 1));
    REQUIRE(ex->MarkerAdd('u', 2));
    wxString text;
    REQUIRE( wxExReplaceMarkers(text, ex));
    text = "'a";
    REQUIRE( wxExReplaceMarkers(text, ex));
    REQUIRE( text == "2");
    text = "'t,'u,therest";
    REQUIRE( wxExReplaceMarkers(text, ex));
    REQUIRE( text == "2,3,therest");
    text = "'z";
    REQUIRE(!wxExReplaceMarkers(text, ex));
    
    delete ex;
  }

  SECTION("wxExSetTextCtrlValue")
  {
  }
  
  SECTION("wxExSort")
  {
    REQUIRE(wxExSort("z\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wxExSort("z\ny\nx\n", STRING_SORT_DESCENDING, 0, "\n") == "z\ny\nx\n");
    REQUIRE(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\nz\n");
    REQUIRE(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING | STRING_SORT_UNIQUE, 0, "\n") == "x\ny\nz\n");
    
    REQUIRE(wxExSort(rect, STRING_SORT_ASCENDING, 3, "\n", 5) == sorted);
  }

  SECTION("wxExSortSelection")
  {
    GetSTC()->SelectNone();
    REQUIRE( wxExSortSelection(GetSTC()));
    GetSTC()->SetText("aaaaa\nbbbbb\nccccc\n");
    GetSTC()->SelectAll();
    REQUIRE( wxExSortSelection(GetSTC()));
    REQUIRE( wxExSortSelection(GetSTC(), STRING_SORT_ASCENDING, 3, 10));
    REQUIRE(!wxExSortSelection(GetSTC(), STRING_SORT_ASCENDING, 20, 10));
    GetSTC()->SelectNone();
    GetSTC()->SetText(rect);
    // force rectangular selection.
    (void)GetSTC()->GetVi().Command("3 ");
    (void)GetSTC()->GetVi().Command("F");
    (void)GetSTC()->GetVi().Command("4j");
    (void)GetSTC()->GetVi().Command("5l");
    REQUIRE( wxExSortSelection(GetSTC(), STRING_SORT_ASCENDING, 3, 5));
    REQUIRE( GetSTC()->GetText() == sorted);
    REQUIRE( wxExSortSelection(GetSTC(), STRING_SORT_DESCENDING, 3, 5));
    REQUIRE( GetSTC()->GetText() != sorted);
  }
  
  SECTION("wxExSkipWhiteSpace")
  {
    REQUIRE( wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  }
  
  SECTION("wxExTranslate")
  {
    REQUIRE(!wxExTranslate(
      "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
  }
      
  SECTION("wxExVCSCommandOnSTC")
  {
    wxExVCSCommand command("status");
    wxExVCSCommandOnSTC(command, wxExLexer("cpp"), GetSTC());
  }
  
  SECTION("wxExVCSExecute")
  {
    // wxExVCSExecute(GetFrame(), 0, std::vector< wxString > {}); // calls dialog
  }
}
