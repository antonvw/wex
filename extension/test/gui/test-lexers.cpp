////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexers.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExLexers", "[stc][lexer]")
{
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello stc");
  AddPane(GetFrame(), stc);
  
  REQUIRE( wxExLexers::Get() != nullptr);
  REQUIRE(!wxExLexers::Get()->GetLexers().empty());
  
  // Test lexer and global macros.
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
    REQUIRE( wxExLexers::Get()->ApplyMacro(
      macro.first.first, macro.first.second) == macro.second);
  }

  // At this moment we have no global properties.
  REQUIRE( wxExLexers::Get()->GetProperties().empty());
  
  wxExLexers::Get()->Apply(stc);

  REQUIRE(!wxExLexers::Get()->GetLexers().empty());

  REQUIRE( wxExLexers::Get()->FindByFileName(
    GetTestFile()).GetScintillaLexer() == "cpp");
    
  REQUIRE( wxExLexers::Get()->FindByName(
    "xxx").GetScintillaLexer().empty());
    
  REQUIRE( wxExLexers::Get()->FindByName(
    "cpp").GetScintillaLexer() == "cpp");
    
  for (const auto& findby : std::vector<std::pair<
    std::string, 
    std::pair<std::string, std::string>>> {
    {"// this is a cpp comment text",{"cpp","cpp"}},
    {"#!/bin/sh",{"bash","sh"}},
    {"#!/bin/csh",{"bash","csh"}},
    {"#!/bin/tcsh",{"bash","tcsh"}},
    {"#!/bin/bash",{"bash","bash"}},
    {"#!/bin/bash\n",{"bash","bash"}},
    {"#!/usr/bin/csh",{"bash","bash"}},
    {"<html>",{"hypertext","hypertext"}},
    {"<?xml",{"hypertext","xml"}}})
  {
    REQUIRE( wxExLexers::Get()->FindByText(
      findby.first).GetScintillaLexer() == findby.second.first);
    REQUIRE( wxExLexers::Get()->FindByText(
      findby.first).GetDisplayLexer() == findby.second.second);
  }
    
  REQUIRE( wxExLexers::Get()->GetFileName().IsOk());

  REQUIRE(!wxExLexers::Get()->GetMacros("global").empty());
  REQUIRE(!wxExLexers::Get()->GetMacros("cpp").empty());
  REQUIRE(!wxExLexers::Get()->GetMacros("pascal").empty());
  REQUIRE( wxExLexers::Get()->GetMacros("XXX").empty());
  
  REQUIRE(!wxExLexers::Get()->GetTheme().empty());
  REQUIRE( wxExLexers::Get()->GetThemeOk());
  REQUIRE(!wxExLexers::Get()->GetThemeMacros().empty());
  REQUIRE( wxExLexers::Get()->GetThemes() > 1);

  wxExLexers::Get()->SetThemeNone();
  REQUIRE( wxExLexers::Get()->GetTheme().empty());
  REQUIRE(!wxExLexers::Get()->GetThemeOk());
  wxExLexers::Get()->RestoreTheme();
  REQUIRE( wxExLexers::Get()->GetTheme() == "torte");
  REQUIRE( wxExLexers::Get()->GetThemeOk());
  
  REQUIRE(!wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(99, -1)));
  REQUIRE( wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0, -1)));
  REQUIRE( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0, -1)));
  
  wxString lexer("cpp");
  wxExLexers::Get()->ShowDialog(GetFrame(), lexer, wxEmptyString, false);
  wxExLexers::Get()->ShowThemeDialog(GetFrame(), wxEmptyString, false);
  
  REQUIRE(!wxExLexers::Get()->GetKeywords("cpp").empty());
  REQUIRE(!wxExLexers::Get()->GetKeywords("csh").empty());
  REQUIRE( wxExLexers::Get()->GetKeywords("xxx").empty());
  REQUIRE( wxExLexers::Get()->GetKeywords(wxEmptyString).empty());

  REQUIRE( wxExLexers::Get()->LoadDocument());
  
  wxExLexers::Get()->ApplyGlobalStyles(stc);
  wxExLexers::Get()->Apply(stc);
}
