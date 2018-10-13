////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wex::tension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/extension/dir.h>
#include <wx/extension/frd.h>
#include <wx/extension/tostring.h>
#include <wx/extension/tool.h>
#include "test.h"

TEST_CASE("wex::report")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  wex::listview* report = new wex::listview(wex::listview_data().Type(wex::LISTVIEW_FIND));
  
  AddPane(GetFrame(), report);
    
  const auto files = wex::get_all_files(
    wex::path("../../../extension/test/gui-report"), 
    "*.cpp", 
    wex::DIR_FILES | wex::DIR_DIRS);
  
  REQUIRE(files.size() > 5);
    
  wex::find_replace_data* frd = wex::find_replace_data::Get(); 
  
  // This string should occur only once, that is here!
  frd->SetUseRegEx(false);
  frd->SetFindString("@@@@@@@@@@@@@@@@@@@");
  
  REQUIRE(GetFrame()->FindInFiles(
    files, 
    wex::ID_TOOL_REPORT_FIND, 
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
    files, 
    wex::ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  REQUIRE(milli.count() < 1500);

#ifdef __UNIX__
#ifndef __WXOSX__
  // Each other file has one author (files.GetCount()), and the one that is already 
  // present on the list because of the first FindInFiles.
  REQUIRE(report->GetItemCount() == files.size() + 2);
#endif
#endif
}
