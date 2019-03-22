////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/dir.h>
#include <wex/frd.h>
#include <wex/tostring.h>
#include <wex/tool.h>
#include "test.h"

TEST_CASE("wex::report")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  wex::listview* report = new wex::listview(
    wex::listview_data().type(wex::listview_data::FIND));
  
  wex::test::add_pane(frame(), report);
    
  const auto files = wex::get_all_files(
    wex::path("../../../extension/test/gui-report"), 
    "*.cpp", std::string(),
    wex::dir::type_t().set());
  
  REQUIRE(files.size() > 5);
    
  wex::find_replace_data* frd = wex::find_replace_data::get(); 
  
  // This string should occur only once, that is here!
  frd->set_use_regex(false);
  frd->set_find_string("@@@@@@@@@@@@@@@@@@@");
  
  REQUIRE(frame()->find_in_files(
    files, 
    wex::ID_TOOL_REPORT_FIND, 
    false, 
    report));
  
#ifdef __UNIX__    
  REQUIRE(report->GetItemCount() == 1);
#endif
  
  frd->set_find_string("Author:");
  
  const auto start = std::chrono::system_clock::now();

  REQUIRE(frame()->find_in_files(
    files, 
    wex::ID_TOOL_REPORT_FIND, 
    false, 
    report));
    
  const auto milli = std::chrono::duration_cast<
    std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
  
  REQUIRE(milli.count() < 1500);

#ifdef __UNIX__
  // Each other file has one author (files.GetCount()), and the one that is already 
  // present on the list because of the first find_in_files.
  REQUIRE(report->GetItemCount() == files.size() + 2);
#endif
}
