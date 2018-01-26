////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/filehistory.h>
#include <wx/extension/util.h>
#include <wx/extension/ex.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/vi-macros.h>
#include "test.h"

TEST_CASE("wxEx")
{
  std::list < std::string > l{"x","y","z"};
  std::vector<int> cs{'(',')','{','<','>'};
    
  const std::string rect("\
012z45678901234567890\n\
123y56789012345678901\n\
234x67890123456789012\n\
345a78901234567890123\n\
456b89012345678901234\n");

  const std::string sorted("\
012a78908901234567890\n\
123b89019012345678901\n\
234x67890123456789012\n\
345y56781234567890123\n\
456z45672345678901234\n");

  SUBCASE("wxExAfter")
  {
    REQUIRE( wxExAfter("nospace", ' ', false) == "nospace");
    REQUIRE( wxExAfter("nospace", ' ', true) == "nospace");
    REQUIRE( wxExAfter("some space and more", ' ', false) == "more");
    REQUIRE( wxExAfter("some space and more", ' ', true) == "space and more");
    REQUIRE( wxExAfter("some space and more", 'm', false) == "ore");
  }

  SUBCASE("wxExAlignText")
  {
    REQUIRE( wxExAlignText("test", "header", true, true,
      wxExLexers::Get()->FindByName("cpp")).size() 
        == wxString("// headertest").size());
  }
      
  SUBCASE("wxExAutoComplete")
  {
    REQUIRE( wxExViMacros::LoadDocument());
    std::string s;
    REQUIRE(!wxExAutoComplete("xxxx", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE(!wxExAutoComplete("Date", // not unique!
      GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( wxExAutoComplete("Datet", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( s == "Datetime");
  }
  
  SUBCASE("wxExAutoCompleteFileName")
  {
    std::string expansion;
    std::vector<std::string> v;
    REQUIRE( wxExAutoCompleteFileName("te", expansion, v));
    REQUIRE( expansion == "st");
    REQUIRE(!wxExAutoCompleteFileName("XX", expansion, v));
    REQUIRE( expansion == "st");
    
#ifdef __UNIX__
#ifndef __WXOSX__    
    REQUIRE( wxExAutoCompleteFileName("/usr/include/s", expansion, v));
    REQUIRE( wxExAutoCompleteFileName("../../../extension/src/v", expansion, v));
    // It is not clear whether ~ is relative or absolute...
    //REQUIRE( wxExAutoCompleteFileName("~/", expansion, v));
#endif    
#endif
  }
  
  SUBCASE("wxExBrowserSearch")
  {
    // Causes travis to hang.
    // REQUIRE( wxExBrowserSearch("test"));
  }

  SUBCASE("wxExClipboardAdd")
  {
    REQUIRE( wxExClipboardAdd("test"));
  }
  
  SUBCASE("wxExClipboardGet")
  {
    REQUIRE( wxExClipboardGet() == "test");
  }
  
  SUBCASE("wxExComboBoxAs")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
#ifndef __WXOSX__
    AddPane(GetFrame(), cb);
#endif
    wxExComboBoxAs<const std::list < std::string >>(cb, l);
  }
  
  SUBCASE("wxExComboBoxFromList")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
#ifndef __WXOSX__
    AddPane(GetFrame(), cb);
#endif
    wxExComboBoxFromList(cb, l);
    REQUIRE( cb->GetCount() == 3);
  }
  
  SUBCASE("wxExCompareFile")
  {
  }
  
  SUBCASE("wxExConfigDir")
  {
#ifdef __WXMSW__
    REQUIRE(!wxExConfigDir().empty());
#else
    REQUIRE(wxExConfigDir().find(".config") != std::string::npos);
#endif
  }
  
  SUBCASE("wxExConfigFirstOf")
  {
    wxExConfigFirstOf("xxxx");
  }
  
  SUBCASE("wxExConfigFirstOfWrite")
  {
    REQUIRE( wxExConfigFirstOfWrite("xxxx", "zz") == "zz");
  }
  
  SUBCASE("wxExEllipsed")
  {
    REQUIRE( wxExEllipsed("xxx").find("...") != std::string::npos);
  }
  
  SUBCASE("wxExFirstOf")
  {
    REQUIRE( wxExFirstOf("this is ok", "x") == std::string());
    REQUIRE( wxExFirstOf("this is ok", " ;,") == "is ok");
    REQUIRE( wxExFirstOf("this is ok", " ;,i") == "s is ok");
    REQUIRE( wxExFirstOf("this is ok", " ;,i", std::string::npos, FIRST_OF_FROM_END) == "ok");
    REQUIRE( wxExFirstOf("this is ok", " ", 0, FIRST_OF_BEFORE) == "this");
    REQUIRE( wxExFirstOf("this is ok", "x", 0, FIRST_OF_BEFORE) == "this is ok");
  }

  SUBCASE("wxExGetEndOfText")
  {
    REQUIRE( wxExGetEndOfText("test", 3).size() == 3);
    REQUIRE( wxExGetEndOfText("testtest", 3).size() == 3);
  }
  
  SUBCASE("wxExGetFieldSeparator")
  {
    REQUIRE((wxExGetFieldSeparator() != 'a'));
  }

  SUBCASE("wxExGetFindResult")
  {
    REQUIRE( wxExGetFindResult("test", true, true).find("test") != std::string::npos);
    REQUIRE( wxExGetFindResult("test", true, false).find("test") != std::string::npos);
    REQUIRE( wxExGetFindResult("test", false, true).find("test") != std::string::npos);
    REQUIRE( wxExGetFindResult("test", false, false).find("test") != std::string::npos);
    
    REQUIRE( wxExGetFindResult("%d", true, true).find("%d") != std::string::npos);
    REQUIRE( wxExGetFindResult("%d", true, false).find("%d") != std::string::npos);
    REQUIRE( wxExGetFindResult("%d", false, true).find("%d") != std::string::npos);
    REQUIRE( wxExGetFindResult("%d", false, false).find("%d") != std::string::npos);
  }
  
  SUBCASE("wxExGetIconID")
  {
    REQUIRE( wxExGetIconID( GetTestPath("test.h")) != -1);
  }

  SUBCASE("wxExGetNumberOfLines  ")
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
  
  SUBCASE("wxExGetStringSet")
  {
    REQUIRE( wxExGetStringSet({"one", "two", "three"}) == "one three two ");
    REQUIRE( wxExGetStringSet({"one", "two", "three"}, 4) == "three ");
  }

  SUBCASE("wxExGetWord")
  {
  }
  
  SUBCASE("wxExIsBrace")
  {
    for (const auto& c : cs)
    {
      REQUIRE( wxExIsBrace(c));
    }
    
    REQUIRE(!wxExIsBrace('a'));
  }

  SUBCASE("wxExIsCodewordSeparator")
  {
    cs.insert(cs.end(), {',',';',':','@'});
    
    for (const auto& c : cs)
    {
      REQUIRE( wxExIsCodewordSeparator(c));
    }
    
    REQUIRE(!wxExIsCodewordSeparator('x'));
  }
  
  SUBCASE("wxExListFromConfig")
  {
    REQUIRE( wxExListFromConfig("xxx").size() == 0);
  }
  
  SUBCASE("wxExListToConfig")
  {
    l.clear();
    l.emplace_back("1");
    l.emplace_back("2");
    wxExListToConfig(l, "list_items");
    REQUIRE( l.size() == 2);
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("1"));
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("2"));
  }
  
  SUBCASE("wxExLogStatus")
  {
    wxExLogStatus( GetTestPath("test.h"));
    wxExLogStatus( std::string("hello world") );
  }

#ifdef __UNIX__
  SUBCASE("wxExMake")
  {
    wxExPath cwd; // as /usr/bin/git changes wd
    REQUIRE( wxExMake(wxExPath("xxx")) != -1);
    REQUIRE( wxExMake(wxExPath("make.tst")) != -1);
    REQUIRE( wxExMake(wxExPath("/usr/bin/git")) != -1);
  }
#endif
  
  SUBCASE("wxExMatch")
  {
    std::vector<std::string> v;
    REQUIRE( wxExMatch("hllo", "hello world", v) == -1);
    REQUIRE( wxExMatch("hello", "hello world", v) == 0);
    REQUIRE( wxExMatch("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wxExMatch("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wxExMatch(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE( wxExMatch("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }
  
  SUBCASE("wxExMatchesOneOf")
  {
    REQUIRE(!wxExMatchesOneOf("test.txt", "*.cpp"));
    REQUIRE( wxExMatchesOneOf("test.txt", "*.txt"));
    REQUIRE( wxExMatchesOneOf("test.txt", "*.cpp;*.txt"));
  }
  
  SUBCASE("wxExNodeProperties")
  {
  }
  
  SUBCASE("wxExNodeStyles")
  {
  }
  
  SUBCASE("wxExOpenFiles")
  {
    REQUIRE( wxExOpenFiles(GetFrame(), std::vector<wxExPath>()) == 0);
    REQUIRE( wxExOpenFiles(GetFrame(), std::vector<wxExPath> {
      GetTestPath("test.h").Path(), "test.cpp", "*xxxxxx*.cpp"}) == 2);
    REQUIRE( wxExOpenFiles(GetFrame(), 
      std::vector<wxExPath> {GetTestPath("test.h").Path()}) == 1);
    REQUIRE( 
      wxExOpenFiles(GetFrame(), std::vector<wxExPath> {"../../data/menus.xml"}) == 1);
  }

  SUBCASE("wxExOpenFilesDialog")
  {
  }
  
  SUBCASE("wxExPrintCaption")
  {
    REQUIRE( wxExPrintCaption(wxExPath("test")).find("test") != std::string::npos);
  }
  
  SUBCASE("wxExPrintFooter")
  {
    REQUIRE( wxExPrintFooter().find("@") != std::string::npos);
  }
  
  SUBCASE("wxExPrintHeader")
  {
    REQUIRE( wxExPrintHeader(GetTestPath("test.h")).find("test") != std::string::npos);
  }
  
  SUBCASE("wxExMarkerAndRegisterExpansion")
  {
    GetSTC()->SetText("this is some text");
    wxExEx* ex = new wxExEx(GetSTC());
    std::string command("xxx");
    REQUIRE(!wxExMarkerAndRegisterExpansion(nullptr, command));
    REQUIRE( wxExMarkerAndRegisterExpansion(ex, command));
    command = "'yxxx";
    REQUIRE(!wxExMarkerAndRegisterExpansion(ex, command));
    wxExClipboardAdd("yanked");
    command = "this is * end";
    REQUIRE( wxExMarkerAndRegisterExpansion(ex, command));
#ifndef __WXMSW__    
    REQUIRE( command == "this is yanked end");
#endif
  }
  
  SUBCASE("wxExQuoted")
  {
    REQUIRE( wxExQuoted("test") == "'test'");
    REQUIRE( wxExQuoted("%d") == "'%d'");
    REQUIRE( wxExQuoted(wxExSkipWhiteSpace(" %d ")) == "'%d'");
  }
  
  SUBCASE("wxExReplaceAll")
  {
    int match_pos;
    const std::string org("test x y z x y z");
    std::string text(org);

    REQUIRE( wxExReplaceAll(text, "x", "aha", &match_pos) == 2);
    REQUIRE( match_pos == 5);

    text = org;
    REQUIRE( wxExReplaceAll(text, "xy", "aha", &match_pos) == 0);
    REQUIRE( match_pos == 5);
  }

#ifdef __UNIX__
  SUBCASE("wxExShellExpansion")
  {
    std::string command("xxx `pwd` `pwd`");
    REQUIRE( wxExShellExpansion(command));
    REQUIRE( command.find("`") == std::string::npos);
    command = "no quotes";
    REQUIRE( wxExShellExpansion(command));
    REQUIRE( command == "no quotes");
    command = "illegal process `xyz`";
    REQUIRE(!wxExShellExpansion(command));
    REQUIRE( command == "illegal process `xyz`");
  }
#endif
  
  SUBCASE("wxExSort")
  {
    REQUIRE(wxExSort("z\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wxExSort("z\ny\nx\n", STRING_SORT_DESCENDING, 0, "\n") == "z\ny\nx\n");
    REQUIRE(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\nz\n");
    REQUIRE(wxExSort("z\nz\ny\nx\n", STRING_SORT_ASCENDING | STRING_SORT_UNIQUE, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wxExSort(rect, STRING_SORT_ASCENDING, 3, "\n", 5) == sorted);
  }

  SUBCASE("wxExSortSelection")
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
    (void)GetSTC()->GetVi().Command("K");
    (void)GetSTC()->GetVi().Command("4j");
    (void)GetSTC()->GetVi().Command("5l");
    REQUIRE( wxExSortSelection(GetSTC(), STRING_SORT_ASCENDING, 3, 5));
#ifdef __WXGTK__
    REQUIRE( wxExSkipWhiteSpace(GetSTC()->GetText().ToStdString()) == wxExSkipWhiteSpace(sorted));
#endif
    REQUIRE( wxExSortSelection(GetSTC(), STRING_SORT_DESCENDING, 3, 5));
    REQUIRE( GetSTC()->GetText() != sorted);
  }
  
  SUBCASE("wxExSkipWhiteSpace")
  {
    REQUIRE( wxExSkipWhiteSpace("\n\tt \n    es   t\n", SKIP_ALL) == "t es t");
    REQUIRE( wxExSkipWhiteSpace("\n\tt \n    es   t\n", SKIP_LEFT) == "t \n    es   t\n");
    REQUIRE( wxExSkipWhiteSpace("\n\tt \n    es   t\n", SKIP_RIGHT) == "\n\tt \n    es   t");
    REQUIRE( wxExSkipWhiteSpace("\n\tt \n    es   t\n", SKIP_BOTH) == "t \n    es   t");
  }
  
  SUBCASE("wxExTranslate")
  {
    REQUIRE(wxExTranslate(
      "hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") == std::string::npos);
  }

  SUBCASE("wxExTypeToValue")
  {
    REQUIRE( wxExTypeToValue<int>("100").get() == 100);
    REQUIRE( wxExTypeToValue<int>("A").get() == 65);
    REQUIRE( wxExTypeToValue<int>(100).get() == 100);
    REQUIRE( wxExTypeToValue<int>(1).getString() == "ctrl-A");
    REQUIRE( wxExTypeToValue<int>("100").getString() == "100");
    REQUIRE( wxExTypeToValue<int>("xxx").getString() == "xxx");
    REQUIRE( wxExTypeToValue<std::string>("100").get() == "100");
    REQUIRE( wxExTypeToValue<std::string>("100").getString() == "100");
  }

  SUBCASE("wxExVCSCommandOnSTC")
  {
    wxExVCSCommand command("status");
    wxExVCSCommandOnSTC(command, wxExLexer(GetSTC(), "cpp"), GetSTC());
    wxExVCSCommandOnSTC(command, wxExLexer(nullptr, "cpp"), GetSTC());
    wxExVCSCommandOnSTC(command, wxExLexer(), GetSTC());
  }
  
  SUBCASE("wxExVCSExecute")
  {
    // wxExVCSExecute(GetFrame(), 0, std::vector< wxString > {}); // calls dialog
  }

  SUBCASE("wxExXmlError")
  {
    wxExPath fn("xml-err.xml");
    pugi::xml_parse_result pr;
    wxExXmlError(fn, &pr);
  }
}
