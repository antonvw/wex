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

void wxExGuiTestFixture::testLexers()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  
  CPPUNIT_ASSERT( wxExLexers::Get() != NULL);
  
  // Test global macro.
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("XXX") == "XXX");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("mark_lcorner") == "10");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("mark_circle") == "0");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("iv_none") == "0");
  
  // Test lexer macro.
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "asm") == "2");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "cpp") == "4");
  
  // At this moment we have no global properties.
  CPPUNIT_ASSERT( wxExLexers::Get()->GetProperties().empty());
  
  wxExLexers::Get()->ApplyMarkers(stc);
  wxExLexers::Get()->ApplyProperties(stc);

  CPPUNIT_ASSERT(!wxExLexers::Get()->BuildWildCards(  GetTestFile()).empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->FindByFileName(
    GetTestFile()).GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "cpp").GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "// this is a cpp comment text").GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/sh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/sh").GetDisplayLexer() == "sh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/csh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/csh").GetDisplayLexer() == "csh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/tcsh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/tcsh").GetDisplayLexer() == "tcsh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/bash").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/usr/bin/csh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "<html>").GetScintillaLexer() == "hypertext");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "<?xml").GetScintillaLexer() == "hypertext");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "xxx").GetScintillaLexer().empty());
    
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().ContainsDefaultStyle());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsOk());

  CPPUNIT_ASSERT( wxExLexers::Get()->GetFileName().IsOk());

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("global").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("pascal").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetMacros("XXX").empty());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeMacros().empty());

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
}
