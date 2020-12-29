////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/indicator.h>
#include <wex/lexers.h>
#include <wex/stc.h>

TEST_CASE("wex::stc")
{
  auto* stc = get_stc();
  stc->get_vi().command("\x1b");
  wex::config(_("stc.Wrap scan")).set(true);

  SUBCASE("config_dialog")
  {
    wex::stc::config_dialog(wex::data::window().button(wxCANCEL | wxAPPLY));
  }

  SUBCASE("text")
  {
    stc->set_text("hello stc");
    REQUIRE(stc->get_text() == "hello stc");

    stc->add_text(" added");
    REQUIRE(stc->get_text().find("added") != std::string::npos);
  }

  SUBCASE("find")
  {
    stc->set_text("hello stc and more text");
    REQUIRE(stc->find_next(std::string("hello")));
    REQUIRE(stc->get_word_at_pos(0) == "hello");

    REQUIRE(!stc->find_next(std::string("%d")));
    REQUIRE(!stc->find_next(std::string("%ld")));
    REQUIRE(!stc->find_next(std::string("%q")));

    REQUIRE(stc->find_next(std::string("hello"), wxSTC_FIND_WHOLEWORD));
    REQUIRE(!stc->find_next(std::string("HELLO"), wxSTC_FIND_MATCHCASE));
    REQUIRE((stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE) > 0);

    wex::find_replace_data::get()->set_regex(false);
    wex::find_replace_data::get()->set_match_case(false);
    REQUIRE(stc->find_next(std::string("HELLO"))); // uses flags from frd
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

    REQUIRE(!stc->set_indicator(wex::indicator(4, 5), 100, 200));
    wex::find_replace_data::get()->set_match_case(false);
    stc->set_search_flags(-1);
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));

    REQUIRE(stc->CanCut());
    stc->Copy();
    REQUIRE(stc->CanPaste());

    stc->DocumentStart();
    wex::find_replace_data::get()->set_match_word(false);
    REQUIRE(stc->find_next(std::string("more text")));
    REQUIRE(stc->get_find_string() == "more text");
    REQUIRE(stc->replace_all("more", "less") == 1);
    REQUIRE(stc->replace_all("more", "less") == 0);
    REQUIRE(!stc->find_next(std::string("more text")));
    stc->SelectNone();
    REQUIRE(!stc->find_next());
    REQUIRE(stc->find_next(std::string("less text")));
    REQUIRE(stc->replace_next("less text", ""));
    REQUIRE(!stc->replace_next());
    REQUIRE(!stc->find_next(std::string("less text")));
    REQUIRE(stc->get_find_string() != "less text");
    REQUIRE(stc->replace_all("%", "percent") == 0);
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

  SUBCASE("lexers")
  {
    for (const auto& l : wex::lexers::get()->get_lexers())
    {
      if (!l.scintilla_lexer().empty())
      {
        CAPTURE(l.scintilla_lexer());
        wex::lexer one(l.scintilla_lexer());
        REQUIRE(one.is_ok());
#ifndef __WXMSW__
        wex::lexer two(stc);
        REQUIRE(two.set(one));
        REQUIRE(two.is_ok());
#endif
      }
    }

    REQUIRE(!wex::lexer().is_ok());
    REQUIRE(!wex::lexer(" cpp").is_ok());
    REQUIRE(!wex::lexer("cpp ").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());

    stc->get_lexer().set("cpp");
    stc->open(wex::test::get_path());
    wex::lexers::get()->apply_global_styles(stc);
    wex::lexers::get()->apply(stc);
    wex::lexers::get()->apply_margin_text_style(
      stc,
      30,
      wex::lexers::margin_style_t::DAY);

    stc->get_file().reset_contents_changed();
  }

  SUBCASE("binary")
  {
    // do the same test as with wex::file in core for a binary file
    REQUIRE(stc->open(wex::test::get_path("test.bin")));
    REQUIRE(stc->data().flags() == 0);
    const auto& buffer = stc->get_text();
    REQUIRE(buffer.length() == 40);
  }

  SUBCASE("contents_changed")
  {
    stc->SetText("added text");
    REQUIRE(stc->get_text().find("added text") != std::string::npos);
    REQUIRE(stc->get_file().is_contents_changed());
    stc->get_file().reset_contents_changed();
    REQUIRE(!stc->get_file().is_contents_changed());

    stc->AppendText("more text");
    REQUIRE(stc->get_text() != "hello stc");
  }

  SUBCASE("marker") { REQUIRE(stc->marker_delete_all_change()); }

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

  SUBCASE("coverage")
  {
    stc->get_lexer().set("cpp");
    stc->Clear();
    stc->clear();
    stc->config_get();
    stc->Cut();
    stc->fold();
    wex::config(_("stc.Auto fold")).set(3);
    stc->fold(true);
    stc->Paste();
    stc->process_char(5);
    stc->properties_message();
    stc->reset_margins();
    stc->SelectNone();
    stc->sync(false);
    stc->sync(true);
    stc->Undo();
    stc->auto_complete().use(true);
    stc->auto_complete().use(false);
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

  SUBCASE("eol") { REQUIRE(!stc->eol().empty()); }

  SUBCASE("position")
  {
    stc->position_restore();
    stc->position_save();
    REQUIRE(stc->position_restore());
  }

  SUBCASE("auto_indentation")
  {
    // first test auto indentation on next line
    wex::config(_("stc.Auto indent")).set(3);
    REQUIRE(wex::config(_("stc.Auto indent")).get(3) == 3);
    stc->set_text("  \n  line with indentation");
    stc->DocumentEnd();
    REQUIRE(!stc->auto_indentation('x'));
    REQUIRE(stc->get_text() == "  \n  line with indentation");
    REQUIRE(stc->get_line_count() == 2);
    stc->SetEOLMode(wxSTC_EOL_CR);
    REQUIRE(stc->auto_indentation(stc->eol().front()));

    // the \n is not added, but indentation does
    REQUIRE(stc->get_text() == "  \n  line with indentation");
    REQUIRE(stc->get_line_count() == 2);
    // test auto indentation for level change
    REQUIRE(stc->get_lexer().set("cpp"));
    stc->set_text("\nif ()\n{\n");
    stc->DocumentEnd();
  }

  SUBCASE("link") { REQUIRE(!stc->link_open()); }

  SUBCASE("hex")
  {
    stc->get_hexmode().set(true);
    REQUIRE(stc->is_hexmode());
    stc->get_hexmode().append_text("in hex mode");
    stc->get_hexmode().set(false);
  }

  SUBCASE("open")
  {
    wex::stc stc(wex::test::get_path("test.h"));
    REQUIRE(stc.get_filename().string().find("test.h") != std::string::npos);
    REQUIRE(stc.open(wex::test::get_path("test.h")));
    REQUIRE(!stc.open("XXX"));
  }

  SUBCASE("complete")
  {
    REQUIRE(stc->get_lexer().set("xml"));
    stc->get_vi().command("i<xxxx>");
    stc->get_vi().command("\x1b");
  }

  SUBCASE("popup") { REQUIRE(stc->get_lexer().set("cpp")); }
}
