////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/dir.h>
#include <wex/frd.h>
#include <wex/tool.h>
#include <wex/tostring.h>

#include "test.h"

void find_in_files(
  const std::vector<std::string>& files,
  wex::del::listview*             lv)
{
  REQUIRE(del_frame()->find_in_files(
    wex::to_vector_path(files).get(),
    wex::ID_TOOL_REPORT_FIND,
    false,
    lv));
}

TEST_CASE("wex::del")
{
  wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  auto* report = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::FIND));

  del_frame()->pane_add(report);

  const auto files = wex::get_all_files(
    std::string("../../test/del"),
    wex::data::dir().file_spec("*.cpp"));

  REQUIRE(files.size() > 5);

  wex::find_replace_data* frd = wex::find_replace_data::get();

  // This string should occur only once, that is here!
  frd->set_regex(false);
  frd->set_find_string("@@@@@@@@@@@@@@@@@@@");

  find_in_files(files, report);

#ifdef __UNIX__
  REQUIRE(report->GetItemCount() == 1);
#endif

  frd->set_find_string("Author:");

  const auto start = std::chrono::system_clock::now();

  find_in_files(files, report);

  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start);

  REQUIRE(milli.count() < 1500);

#ifdef __UNIX__
  // Each other file has one author (files.GetCount()), and the one that is
  // already present on the list because of the first find_in_files.
  REQUIRE(report->GetItemCount() == files.size() + 2);
#endif
}
