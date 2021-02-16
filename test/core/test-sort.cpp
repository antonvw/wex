////////////////////////////////////////////////////////////////////////////////
// Name:      test-sort.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "../test.h"
#include <boost/algorithm/string.hpp>
#include <wex/sort.h>
#include <wex/stc-core.h>

class stc : public wex::core::stc
{
public:
  stc() { Create(wxTheApp->GetTopWindow(), -1); };
  const std::string eol() const { return std::string(); };
  void              fold(bool fold_all = false) { ; };

  const wex::path&  get_filename() const { return m_path; };
  const std::string get_selected_text() const
  {
    return const_cast<stc*>(this)->GetSelectedText();
  };
  bool is_hexmode() const { return false; };
  bool is_visual() const { return false; };
  void properties_message(wex::path::status_t flags = 0) { ; };
  void reset_margins(margin_t type = margin_t().set()) { ; };
  bool set_hexmode(bool on) { return false; };
  bool set_indicator(const wex::indicator& indicator, int start, int end)
  {
    return false;
  };
  void set_search_flags(int flags) { ; };
  bool vi_command(const std::string& command) { return false; };

  bool find(const std::string& text, int find_flags = -1, bool find_next = true)
  {
    return false;
  };

  int  get_line_count() const { return 0; };
  int  get_line_count_request() { return 0; };
  void goto_line(int line) { ; };

private:
  wex::path m_path;
};

TEST_CASE("wex::sort" * doctest::may_fail())
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

  SUBCASE("str")
  {
    REQUIRE(wex::sort().str("z\ny\nx\n", "\n") == "x\ny\nz\n");
    REQUIRE(
      wex::sort(wex::sort::sort_t().set(wex::sort::SORT_DESCENDING))
        .str("z\ny\nx\n", "\n") == "z\ny\nx\n");
    REQUIRE(wex::sort().str("z\nz\ny\nx\n", "\n") == "x\ny\nz\nz\n");
    REQUIRE(
      wex::sort(wex::sort::sort_t().set(wex::sort::SORT_UNIQUE))
        .str("z\nz\ny\nx\n", "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort(0, 3, 5).str(rect, "\n") == sorted);
  }

  static wxFrame* frame = nullptr;

  if (frame == nullptr)
  {
    frame = new wxFrame(nullptr, -1, "core test");
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
