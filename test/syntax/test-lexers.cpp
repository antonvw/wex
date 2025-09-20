////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexers.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/lexers.h>
#include <wex/test/test.h>

// see also test-stc

TEST_CASE("wex::lexers")
{
  SECTION("get")
  {
    auto* l = wex::lexers::set(nullptr);
    REQUIRE(l != nullptr);
    REQUIRE(l->get_lexers().size() > 1);
    REQUIRE(wex::lexers::get(false) == nullptr);

    wex::lexers::is_initial_load(false);
    REQUIRE(wex::lexers::get() != l);
    REQUIRE(wex::lexers::get()->get_lexers().size() == 1);
    REQUIRE(!wex::lexers::get()->is_loaded());
    REQUIRE(wex::lexers::get()->marker_max_no_used() == -1);
    REQUIRE(!wex::lexers::get()->indicator_is_loaded(wex::indicator(0)));
    REQUIRE(!wex::lexers::get()->marker_is_loaded(wex::marker(0)));
    REQUIRE(!wex::lexers::get()->get_marker(wex::marker(0)).is_ok());

    wex::lexers::is_initial_load(true);
    auto* m = wex::lexers::set(nullptr);
    REQUIRE(m != nullptr);
    REQUIRE(wex::lexers::get()->get_lexers().size() > 1);
    REQUIRE(wex::lexers::get()->is_loaded());

    delete l;
    delete m;
  }

  SECTION("apply_default_style")
  {
    wex::lexers::get()->apply_default_style(
      [=](const std::string& back)
      {
      },
      [=](const std::string& fore)
      {
      });
  }

  SECTION("apply_macro")
  {
    for (const auto& macro : std::vector<
           std::pair<std::pair<std::string, std::string>, std::string>>{
           {{"number", "asm"}, "2"},
           {{"number", "cpp"}, "4"},
           {{"XXX", "global"}, "XXX"},
           {{"mark_lcorner", "global"}, "10"},
           {{"mark_circle", "global"}, "0"},
           {{"style_textmargin", "global"}, "55"},
           {{"iv_none", "global"}, "0"}})
    {
      const auto& result(
        wex::lexers::get()->apply_macro(macro.first.first, macro.first.second));

      CAPTURE(result);
      REQUIRE(result == macro.second);
    }
  }

  SECTION("find_by")
  {
    REQUIRE(
      wex::lexers::get()
        ->find_by_filename(wex::test::get_path("test.h").filename())
        .scintilla_lexer() == "cpp");

    REQUIRE(wex::lexers::get()->find("xxx").scintilla_lexer().empty());

    REQUIRE(wex::lexers::get()->find("cpp").scintilla_lexer() == "cpp");

    for (const auto& findby : std::vector<
           std::pair<std::string, std::pair<std::string, std::string>>>{
           {"// this is a cpp comment text", {"cpp", "cpp"}},
           {"#!/bin/bash", {"bash", "bash"}},
           {"#!/bin/bash\n", {"bash", "bash"}},
           {"#!/usr/bin/csh", {"bash", "bash"}},
           {"#!/bin/csh", {"bash", "csh"}},
           {"#!/bin/env python", {"python", "python"}},
           {"#!/bin/sh", {"bash", "sh"}},
           {"#!/bin/tcsh", {"bash", "tcsh"}},
           {"<html>", {"hypertext", "hypertext"}},
           {"<?xml", {"hypertext", "xml"}}})
    {
      REQUIRE(
        wex::lexers::get()->find_by_text(findby.first).scintilla_lexer() ==
        findby.second.first);
      REQUIRE(
        wex::lexers::get()->find_by_text(findby.first).display_lexer() ==
        findby.second.second);
    }
  }

  SECTION("keywords")
  {
    REQUIRE(!wex::lexers::get()->keywords("cpp").empty());
    REQUIRE(!wex::lexers::get()->keywords("csh").empty());

    REQUIRE(wex::lexers::get()->keywords("xxx").empty());
    REQUIRE(wex::lexers::get()->keywords(std::string()).empty());
  }

  SECTION("markers")
  {
    REQUIRE(wex::lexers::get()->marker_is_loaded(wex::marker(0)));
    REQUIRE(wex::lexers::get()->marker_max_no_used() > 4);
    REQUIRE(wex::lexers::get()->marker_max_no_used() < wxSTC_MARKNUM_FOLDEREND);
    REQUIRE(wex::lexers::get()->get_marker(wex::marker(0)).is_ok());
  }

  SECTION("properties")
  {
    REQUIRE(wex::lexers::get()->properties().empty());
  }

  SECTION("rest")
  {
    REQUIRE(!wex::lexers::get()->path().empty());

    REQUIRE(!wex::lexers::get()->get_macros("global").empty());
    REQUIRE(!wex::lexers::get()->get_macros("cpp").empty());
    REQUIRE(!wex::lexers::get()->get_macros("pascal").empty());
    REQUIRE(wex::lexers::get()->get_macros("XXX").empty());

    REQUIRE(!wex::lexers::get()->theme().empty());
    REQUIRE(!wex::lexers::get()->theme_macros().empty());
    REQUIRE(wex::lexers::get()->get_themes_size() > 1);

    wex::lexers::get()->clear_theme();
    REQUIRE(wex::lexers::get()->theme().empty());
    wex::lexers::get()->restore_theme();
    REQUIRE(!wex::lexers::get()->theme().empty());

    REQUIRE(!wex::lexers::get()->indicator_is_loaded(wex::indicator(99)));
    REQUIRE(wex::lexers::get()->indicator_is_loaded(wex::indicator(0)));
    REQUIRE(wex::lexers::get()->get_indicator(wex::indicator(0)).is_ok());

    REQUIRE(wxTheColourDatabase->Find("gray 2").IsOk());

    REQUIRE(wex::lexers::get()->load_document());
  }
}
