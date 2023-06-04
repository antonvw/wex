////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/defs.h>
#include <wex/stc/auto-complete.h>
#include <wex/ui/frd.h>

#include "test.h"

void modeline_from_file(const std::string& name)
{
  // Not yet tested.
}

TEST_CASE("wex::stc")
{
  auto* stc = get_stc();
  stc->get_vi().command("\x1b");
  wex::config(_("stc.Wrap scan")).set(true);

  SUBCASE("auto_complete")
  {
    stc->auto_complete()->use(true);
    REQUIRE(stc->auto_complete()->use());

    stc->auto_complete()->use(false);
    REQUIRE(!stc->auto_complete()->use());
  }

  SUBCASE("auto_indentation")
  {
    REQUIRE(!stc->auto_indentation('x'));
  }

  SUBCASE("binary")
  {
    // do the same test as with wex::file in core for a binary file
    REQUIRE(stc->open(wex::test::get_path("test.bin")));
    REQUIRE(stc->data().flags() == 0);
    const auto& buffer = stc->get_text();
    REQUIRE(buffer.length() == 40);
  }

  SUBCASE("config_dialog")
  {
    wex::stc::config_dialog(wex::data::window().button(wxCANCEL | wxAPPLY));
  }

  SUBCASE("contents_changed")
  {
    stc->SetText("added text");
    REQUIRE(stc->get_text().contains("added text"));
    REQUIRE(stc->get_file().is_contents_changed());
    stc->get_file().reset_contents_changed();
    REQUIRE(!stc->get_file().is_contents_changed());

    stc->AppendText("more text");
    REQUIRE(stc->get_text() != "hello stc");
  }

  SUBCASE("coverage")
  {
    REQUIRE(!stc->process_char(WXK_BACK));

    stc->get_lexer().set("cpp");
    stc->Clear();
    stc->clear();
    stc->config_get();
    stc->Cut();
    stc->Paste();
    stc->process_char(5);
    stc->properties_message();
    stc->reset_margins();
    stc->SelectNone();
    stc->sync(false);
    stc->sync(true);
    stc->Undo();
    stc->use_modification_markers(true);
    stc->use_modification_markers(false);

    stc->LineHome();
    stc->LineHomeExtend();
    stc->LineHomeRectExtend();
    stc->LineScrollDownExtend();
    stc->LineScrollDownRectExtend();
    stc->LineScrollUpExtend();
    stc->LineScrollUpRectExtend();
    stc->ParaUpRectExtend();
    stc->ParaDownRectExtend();
    stc->WordLeftRectExtend();
    stc->WordRightRectExtend();
    stc->WordRightEndRectExtend();
  }

  SUBCASE("eol")
  {
    REQUIRE(!stc->eol().empty());
  }

  SUBCASE("find")
  {
    for (const auto mode : {wex::ex::OFF, wex::ex::VISUAL})
    {
      stc->get_vi().use(mode);

      stc->set_text("hello stc and more text");
      REQUIRE(stc->find("hello"));
      REQUIRE(stc->get_word_at_pos(0) == "hello");

      REQUIRE(!stc->find("%d"));
      REQUIRE(!stc->find("%ld"));
      REQUIRE(!stc->find("%q"));

      REQUIRE(stc->find("hello", wxSTC_FIND_WHOLEWORD));
      REQUIRE(!stc->find("hell", wxSTC_FIND_WHOLEWORD));
      REQUIRE(!stc->find("HELLO", wxSTC_FIND_MATCHCASE));

      REQUIRE((stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE) > 0);
      REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_WHOLEWORD) > 0);

      // uses flags from frd
      wex::find_replace_data::get()->set_regex(false);
      wex::find_replace_data::get()->set_match_case(false);
      REQUIRE(stc->find("HELLO"));
      wex::find_replace_data::get()->set_match_word(true);
      REQUIRE(!stc->find("HELL"));
      REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

      wex::find_replace_data::get()->set_match_case(false);
      stc->set_search_flags(-1);
      REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

      REQUIRE(stc->CanCut());
      stc->Copy();
      REQUIRE(stc->CanPaste());

      stc->DocumentStart();
      wex::find_replace_data::get()->set_match_word(false);
      REQUIRE(stc->find("more text"));
      REQUIRE(stc->get_find_string() == "more text");
      REQUIRE(stc->replace_all("more", "less") == 1);
      REQUIRE(stc->replace_all("more", "less") == 0);
      REQUIRE(!stc->find("more text"));
      stc->SelectNone();
      REQUIRE(stc->find("less text"));
      REQUIRE(stc->replace_next("less text", ""));
      REQUIRE(!stc->replace_next());
      REQUIRE(!stc->find("less text"));
      REQUIRE(stc->get_find_string() != "less text");
      REQUIRE(stc->replace_all("%", "percent") == 0);
    }
  }

  SUBCASE("find_next")
  {
    for (const auto mode : {wex::ex::OFF, wex::ex::VISUAL})
    {
      stc->get_vi().use(mode);

      stc->set_text("hello stc and more text");
      wex::find_replace_data::get()->set_find_string("hello");
      wex::find_replace_data::get()->set_match_word(true);
      REQUIRE(stc->find_next(false));

      wex::find_replace_data::get()->set_find_string("hell");
      REQUIRE(!stc->find_next(false));
    }

    wex::find_replace_data::get()->set_match_word(true);
  }

  SUBCASE("hexmode")
  {
    stc->get_hexmode().set(true);
    REQUIRE(stc->is_hexmode());
    stc->get_hexmode().append_text("in hex mode");
    stc->get_hexmode().set(false);

    auto* vi = &stc->get_vi();
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();

    // insert on hexmode document
    stc->SetReadOnly(false);
    REQUIRE(!stc->GetModify());

    stc->get_hexmode().set(true);
    REQUIRE(stc->is_hexmode());
    REQUIRE(!stc->GetModify());
    REQUIRE(vi->command("a"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(!vi->command("xxxxxxxx"));

    REQUIRE(stc->get_hexmode_insert("55", 0));
    REQUIRE(stc->get_hexmode_replace('a'));
    REQUIRE(!stc->get_hexmode_lines("AA").empty());
    REQUIRE(stc->get_hexmode_erase(0, 1));
    REQUIRE(stc->get_hexmode_replace_target("44", false));
    REQUIRE(stc->get_hexmode_sync());

    REQUIRE(vi->command("\x1b"));
    REQUIRE(stc->GetModify());

    stc->get_hexmode().set(false);
    REQUIRE(!stc->is_hexmode());

    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
    REQUIRE(!stc->GetModify());

    stc->SetReadOnly(false);
  }

  SUBCASE("hypertext")
  {
    stc->set_text("");
    REQUIRE(stc->get_lexer().set("xml"));

    event(stc, "i<xxxxx>\x1b");
#ifdef TEST
    // Due to queueing ? only ok i tested separately.
    REQUIRE(stc->get_text() == "<xxxxx></xxxxx>");
#endif
  }

  SUBCASE("lexer")
  {
    stc->set_text("new text");
    REQUIRE(stc->get_lexer().set("cpp"));
    REQUIRE(stc->get_lexer().scintilla_lexer() == "cpp");
    stc->get_lexer().clear();
    REQUIRE(stc->get_lexer().scintilla_lexer().empty());

    wex::lexer lexer;
    lexer.clear();
    REQUIRE(lexer.set("cpp", true));
    REQUIRE(!lexer.set("xyz"));
    REQUIRE(stc->get_lexer().set(lexer));

    wex::lexer lexer_s(stc);
    REQUIRE(!lexer_s.is_ok());
    REQUIRE(lexer_s.apply());

    stc->SetText("// a rust comment");
    REQUIRE(lexer_s.set("rust"));
    REQUIRE(lexer_s.scintilla_lexer() == "rust");
  }

  SUBCASE("link")
  {
    stc->SetText("no link");
#ifdef __WXOSX__
    REQUIRE(!stc->link_open());
#endif
  }

  SUBCASE("margin")
  {
    REQUIRE(stc->get_margin_text_click() == -1);

    stc->show_line_numbers(false);
    REQUIRE(!stc->is_shown_line_numbers());
    stc->show_line_numbers(true);
    REQUIRE(stc->is_shown_line_numbers());
    stc->show_line_numbers(false);
    REQUIRE(!stc->is_shown_line_numbers());
  }

  SUBCASE("marker")
  {
    REQUIRE(stc->marker_delete_all_change());
  }

  SUBCASE("modeline")
  {
    SUBCASE("text")
    {
      const std::string modeline("set ts=120 ec=40 sy=sql sw=4 nu el");
      auto*             stc = new wex::stc(std::string("-- vi: " + modeline));
      frame()->pane_add(stc);

      REQUIRE(stc->get_vi().is_active());
      REQUIRE(stc->GetTabWidth() == 120);
      REQUIRE(stc->GetEdgeColumn() == 40);
      REQUIRE(stc->GetIndent() == 4);
      REQUIRE(stc->get_lexer().scintilla_lexer() == "sql");
    }

    SUBCASE("modeline-vi")
    {
      auto* stc = new wex::stc(std::string("// 	vi: set ts=120 "
                                           "// this is a modeline"));
      frame()->pane_add(stc);
      REQUIRE(stc->GetTabWidth() == 120);
      REQUIRE(stc->get_vi().mode().is_command());
    }

    SUBCASE("modeline-vim")
    {
      auto* stc =
        // see e.g. libre-office-xmlreader.h
        new wex::stc(std::string("// vim: set softtabstop=120 expandtab:"));
      frame()->pane_add(stc);
      REQUIRE(!stc->GetUseTabs());
      REQUIRE(stc->GetTabWidth() == 120);
      REQUIRE(stc->get_vi().mode().is_command());
    }

    modeline_from_file("test-modeline.txt");

    modeline_from_file("test-modeline2.txt");
  }

  SUBCASE("open")
  {
    wex::stc stc(wex::test::get_path("test.h"));
    REQUIRE(stc.path().string().contains("test.h"));
    REQUIRE(stc.open(wex::test::get_path("test.h")));
    REQUIRE(!stc.open(wex::path("XXX")));
  }

  SUBCASE("popup")
  {
    REQUIRE(stc->get_lexer().set("cpp"));
  }

  SUBCASE("shift-double-click")
  {
    stc->set_text("hello stc");
    event(stc, WXK_SHIFT, wxEVT_KEY_DOWN);
    event(stc, WXK_SHIFT, wxEVT_KEY_DOWN);
    // result is not yet checked
  }

  SUBCASE("text")
  {
    stc->set_text("hello stc");
    REQUIRE(stc->get_text() == "hello stc");

    stc->add_text(" added");
    REQUIRE(stc->get_text().contains("added"));
  }

  SUBCASE("vi")
  {
    REQUIRE(stc->vi_command(wex::line_data().command("G")));
    REQUIRE(stc->vi_command_finish(false));
    stc->vi_record("xx");
    REQUIRE(!stc->vi_is_visual());
    REQUIRE(stc->vi_register('c').empty());
    REQUIRE((stc->vi_search_flags() & wxSTC_FIND_REGEXP) > 0);
    REQUIRE(stc->vi_mode().empty());

    REQUIRE(stc->is_visual());
  }
}
