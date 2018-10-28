////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexers.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/lexers.h>
#include <wex/managedframe.h>
#include <wex/path.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::lexers")
{
  SUBCASE("Get")
  {
    REQUIRE( wex::lexers::Get() != nullptr);
    REQUIRE(!wex::lexers::Get()->get().empty());
  }
  
  SUBCASE("lexer and global macros")
  {
    for (const auto& macro : std::vector<
      std::pair<
        std::pair<std::string,std::string>,
        std::string>> {
      {{"number","asm"},"2"},
      {{"number","cpp"},"4"},
      {{"XXX","global"},"XXX"},
      {{"mark_lcorner","global"},"10"},
      {{"mark_circle","global"},"0"},
      {{"iv_none","global"},"0"}})
    {
      REQUIRE( wex::lexers::Get()->ApplyMacro(
        macro.first.first, macro.first.second) == macro.second);
    }
  }

  SUBCASE("Properties")
  {
    REQUIRE( wex::lexers::Get()->GetProperties().empty());
  }
  
  SUBCASE("FindBy")
  {
    REQUIRE( wex::lexers::Get()->FindByFileName(
      GetTestPath("test.h").GetFullName()).GetScintillaLexer() == "cpp");
      
    REQUIRE( wex::lexers::Get()->FindByName(
      "xxx").GetScintillaLexer().empty());
      
    REQUIRE( wex::lexers::Get()->FindByName(
      "cpp").GetScintillaLexer() == "cpp");
    
    for (const auto& findby : std::vector<std::pair<
      std::string, 
      std::pair<std::string, std::string>>> {
      {"// this is a cpp comment text",{"cpp","cpp"}},
      {"#!/bin/bash",{"bash","bash"}},
      {"#!/bin/bash\n",{"bash","bash"}},
      {"#!/usr/bin/csh",{"bash","bash"}},
      {"#!/bin/csh",{"bash","csh"}},
      {"#!/bin/env python",{"python","python"}},
      {"#!/bin/sh",{"bash","sh"}},
      {"#!/bin/tcsh",{"bash","tcsh"}},
      {"<html>",{"hypertext","hypertext"}},
      {"<?xml",{"hypertext","xml"}}})
    {
      REQUIRE( wex::lexers::Get()->FindByText(
        findby.first).GetScintillaLexer() == findby.second.first);
      REQUIRE( wex::lexers::Get()->FindByText(
        findby.first).GetDisplayLexer() == findby.second.second);
    }
  }
    
  SUBCASE("Rest")
  {
    REQUIRE(!wex::lexers::Get()->GetFileName().Path().empty());

    REQUIRE(!wex::lexers::Get()->GetMacros("global").empty());
    REQUIRE(!wex::lexers::Get()->GetMacros("cpp").empty());
    REQUIRE(!wex::lexers::Get()->GetMacros("pascal").empty());
    REQUIRE( wex::lexers::Get()->GetMacros("XXX").empty());
    
    REQUIRE(!wex::lexers::Get()->GetTheme().empty());
    REQUIRE(!wex::lexers::Get()->GetThemeMacros().empty());
    REQUIRE( wex::lexers::Get()->GetThemes() > 1);

    wex::lexers::Get()->ResetTheme();
    REQUIRE( wex::lexers::Get()->GetTheme().empty());
    wex::lexers::Get()->RestoreTheme();
    REQUIRE(!wex::lexers::Get()->GetTheme().empty());
    
    REQUIRE(!wex::lexers::Get()->IndicatorIsLoaded(wex::indicator(99)));
    REQUIRE( wex::lexers::Get()->IndicatorIsLoaded(wex::indicator(0)));
    REQUIRE( wex::lexers::Get()->MarkerIsLoaded(wex::marker(0)));
    REQUIRE( wex::lexers::Get()->GetIndicator(wex::indicator(0)).is_ok());
    REQUIRE( wex::lexers::Get()->GetMarker(wex::marker(0)).is_ok());
    
    wxString lexer("cpp");
    REQUIRE(!wex::lexers::Get()->GetKeywords("cpp").empty());
    REQUIRE(!wex::lexers::Get()->GetKeywords("csh").empty());
    REQUIRE( wex::lexers::Get()->GetKeywords("xxx").empty());
    REQUIRE( wex::lexers::Get()->GetKeywords(std::string()).empty());

    REQUIRE( wex::lexers::Get()->LoadDocument());
    
    wex::lexers::Get()->ApplyGlobalStyles(GetSTC());
    wex::lexers::Get()->Apply(GetSTC());
  }
}
