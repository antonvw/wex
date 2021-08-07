////////////////////////////////////////////////////////////////////////////////
// Name:      test-sort.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/factory/stc.h>
#include <wex/sort.h>

#include "../test.h"

class stc : public wex::factory::stc
{
public:
  stc() { Create(wxTheApp->GetTopWindow(), -1); }

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};

TEST_CASE("wex::sort")
{
  const std::string rect("012z45678901234567890\n"
                         "123y56789012345678901\n"
                         "234x67890123456789012\n"
                         "345a78901234567890123\n"
                         "456b89012345678901234\n");

  const std::string sorted("012a78908901234567890\n"
                           "123b89019012345678901\n"
                           "234x67890123456789012\n"
                           "345y56781234567890123\n"
                           "456z45672345678901234\n");

#ifndef __WXMSW__
  SUBCASE("string")
  {
    REQUIRE(wex::sort().string("z\ny\nx\n", "\n") == "x\ny\nz\n");
    REQUIRE(
      wex::sort(wex::sort::sort_t().set(wex::sort::SORT_DESCENDING))
        .string("z\ny\nx\n", "\n") == "z\ny\nx\n");
    REQUIRE(wex::sort().string("z\nz\ny\nx\n", "\n") == "x\ny\nz\nz\n");
    REQUIRE(
      wex::sort(wex::sort::sort_t().set(wex::sort::SORT_UNIQUE))
        .string("z\nz\ny\nx\n", "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort(0, 3, 5).string(rect, "\n") == sorted);
  }
#endif

  static wxFrame* frame = nullptr;

  if (frame == nullptr)
  {
    frame = new wxFrame(nullptr, -1, "sort test");
    frame->Show();
  }

  auto* s = new stc();

  SUBCASE("selection")
  {
    s->SelectNone();
    REQUIRE(!wex::sort().selection(s));

    s->SetText("aaaaa\nbbbbb\nccccc\n");
    s->SelectAll();
    wxMilliSleep(10);
    REQUIRE(wex::sort().selection(s));
    REQUIRE(wex::sort(0, 3, 10).selection(s));
    REQUIRE(!wex::sort(0, 20, 10).selection(s));
  }

  SUBCASE("block")
  {
    s->SelectNone();
    s->SetText(rect);

    // make a block selection, invoke sort, and check result
    s->CharRight();
    s->CharRight();
    s->CharRight();

    s->LineDownRectExtend();
    s->LineDownRectExtend();
    s->LineDownRectExtend();
    s->LineDownRectExtend();

    s->CharRightRectExtend();
    s->CharRightRectExtend();
    s->CharRightRectExtend();
    s->CharRightRectExtend();
    s->CharRightRectExtend();

    REQUIRE(wex::sort(0, 3, 5).selection(s));

    REQUIRE(
      boost::algorithm::trim_copy(s->GetText().ToStdString()) ==
      boost::algorithm::trim_copy(sorted));
    REQUIRE(wex::sort(wex::sort::sort_t().set(wex::sort::SORT_DESCENDING), 3, 5)
              .selection(s));
    REQUIRE(s->GetText() != sorted);
  }
}
