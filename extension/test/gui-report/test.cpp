////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/dir.h>
#include <wx/extension/frd.h>
#include <wx/extension/tostring.h>
#include <wx/extension/tool.h>
#include "test.h"

TEST_CASE("wxEx")
{
  wxExTool tool(ID_TOOL_REPORT_FIND);

  wxExListView* report = new wxExListView(
    GetFrame(), 
    wxExListView::LIST_FILE);
  
  AddPane(GetFrame(), report);
    
  wxArrayString files;
  
  REQUIRE(wxDir::GetAllFiles(
    "../../../extension/test/gui-report", 
    &files,
    "*.cpp", 
    wxDIR_FILES | wxDIR_DIRS) > 5);
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetUseRegEx(false);
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  REQUIRE(GetFrame()->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
  
#ifdef __UNIX__    
#ifndef __WXOSX__
  REQUIRE(report->GetItemCount() == 1);
#endif
#endif
  
  frd->SetFindString("Author:");
  
  const auto start = std::chrono::system_clock::now();

  REQUIRE(GetFrame()->FindInFiles(
    wxExToVectorString(files).Get(), 
    ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  REQUIRE(milli.count() < 1500);

#ifdef __UNIX__
#ifndef __WXOSX__
  // Each other file has one author (files.GetCount()), and the one that is already 
  // present on the list because of the first FindInFiles.
  REQUIRE(report->GetItemCount() == (
    wxExToVectorString(files).Get().size() + 2));
#endif
#endif
}
