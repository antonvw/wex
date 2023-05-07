////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/dir.h>
#include <wex/ui/frd.h>

#include "test.h"

#include <chrono>

void find_in_files(const std::vector<wex::path>& v, wex::del::listview* lv)
{
  REQUIRE(del_frame()
            ->find_in_files(v, wex::tool(wex::ID_TOOL_REPORT_FIND), false, lv));
}

TEST_CASE("wex::del")
{
  auto* lv = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::FIND));

  del_frame()->pane_add(lv);

  const auto& files = wex::get_all_files(
    wex::path("../../test/del"),
    wex::data::dir().file_spec("*.cpp"));

  REQUIRE(files.size() > 5);

  wex::find_replace_data* frd = wex::find_replace_data::get();

  // This string should occur only once, that is here!
  frd->set_regex(false);
  frd->set_find_string("@@@@@@@@@@@@@@@@@@@");

  find_in_files(files, lv);

  wxYield();

#ifdef __UNIX__
  REQUIRE(lv->GetItemCount() == 1);
#endif

  frd->set_find_string("Author:");

  const auto start = std::chrono::system_clock::now();

  find_in_files(files, lv);

  wxYield();

  const auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start);

  REQUIRE(milli.count() < 1500);

#ifdef __UNIX__
  // Each other file has one author (files.GetCount()), and the one that is
  // already present on the list because of the first find_in_files.
  REQUIRE(lv->GetItemCount() == files.size() + 2);
#endif
}
