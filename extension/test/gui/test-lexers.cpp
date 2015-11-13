////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexers.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void fixture::testLexers()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  AddPane(m_Frame, stc);
  
  CPPUNIT_ASSERT( wxExLexers::Get() != NULL);
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().ContainsDefaultStyle());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsOk());
  
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
    CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro(
      macro.first.first, macro.first.second) == macro.second);
  }

  // At this moment we have no global properties.
  CPPUNIT_ASSERT( wxExLexers::Get()->GetProperties().empty());
  
  wxExLexers::Get()->ApplyMarkers(stc);
  wxExLexers::Get()->ApplyProperties(stc);

  CPPUNIT_ASSERT(!wxExLexers::Get()->BuildWildCards(  GetTestFile()).empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->FindByFileName(
    GetTestFile()).GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "xxx").GetScintillaLexer().empty());
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
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
    CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
      findby.first).GetScintillaLexer() == findby.second.first);
    CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
      findby.first).GetDisplayLexer() == findby.second.second);
  }
    
  CPPUNIT_ASSERT( wxExLexers::Get()->GetFileName().IsOk());

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("global").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("pascal").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetMacros("XXX").empty());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeMacros().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemes() > 1);

  CPPUNIT_ASSERT(!wxExLexers::Get()->SetTheme("xxx"));
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  CPPUNIT_ASSERT( wxExLexers::Get()->SetTheme("torte"));
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme() == "torte");
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  wxExLexers::Get()->SetThemeNone();
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeOk());
  wxExLexers::Get()->RestoreTheme();
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme() == "torte");
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(99, -1)));
  CPPUNIT_ASSERT( wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0, -1)));
  CPPUNIT_ASSERT( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0, -1)));
  
  wxString lexer("cpp");
  wxExLexers::Get()->ShowDialog(m_Frame, lexer, wxEmptyString, false);
  wxExLexers::Get()->ShowThemeDialog(m_Frame, wxEmptyString, false);
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetKeywords("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetKeywords("csh").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetKeywords("xxx").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetKeywords(wxEmptyString).empty());

  CPPUNIT_ASSERT( wxExLexers::Get()->LoadDocument());
  
  wxExLexers::Get()->ApplyGlobalStyles(stc);
  wxExLexers::Get()->ApplyHexStyles(stc);
  wxExLexers::Get()->ApplyIndicators(stc);
}
