////////////////////////////////////////////////////////////////////////////////
// Name:      test-stc.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stc.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/frd.h>
#include <wex/indicator.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::stc")
{
  auto* stc = get_stc();
  stc->get_vi().command("\x1b");
  wex::config(_("stc.Wrap scan")).set(true);
  
  SUBCASE("config_dialog")
  {
    wex::stc::config_dialog(wex::window_data().button(wxCANCEL | wxAPPLY));
  }
  
  SUBCASE("text")
  {
    stc->set_text("hello stc");
    REQUIRE( stc->GetText() == "hello stc");

    stc->add_text(" added");
    REQUIRE( stc->GetText().Contains("added"));
  }
  
  SUBCASE("find")
  {
    stc->set_text("hello stc and more text");
    REQUIRE( stc->find_next(std::string("hello")));
    REQUIRE( stc->get_word_at_pos(0) == "hello");
    
    REQUIRE(!stc->find_next(std::string("%d")));
    REQUIRE(!stc->find_next(std::string("%ld")));
    REQUIRE(!stc->find_next(std::string("%q")));
    
    REQUIRE( stc->find_next(std::string("hello"), wxSTC_FIND_WHOLEWORD));
    REQUIRE(!stc->find_next(std::string("HELLO"), wxSTC_FIND_MATCHCASE));
    REQUIRE((stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE) > 0);
    
    wex::find_replace_data::get()->set_use_regex(false);
    wex::find_replace_data::get()->set_match_case(false);
    REQUIRE( stc->find_next(std::string("HELLO"))); // uses flags from frd
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
    
    REQUIRE(!stc->set_indicator(wex::indicator(4,5), 100, 200));
    wex::find_replace_data::get()->set_match_case(false);
    stc->set_search_flags(-1);
    REQUIRE(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
    
    REQUIRE( stc->CanCut());
    stc->Copy();
    REQUIRE( stc->CanPaste());
    
    stc->DocumentStart();
    wex::find_replace_data::get()->set_match_word(false);
    REQUIRE( stc->find_next(std::string("more text")));
    REQUIRE( stc->get_find_string() == "more text");
    REQUIRE( stc->replace_all("more", "less") == 1);
    REQUIRE( stc->replace_all("more", "less") == 0);
    REQUIRE(!stc->find_next(std::string("more text")));
    stc->SelectNone();
    REQUIRE(!stc->find_next());
    REQUIRE( stc->find_next(std::string("less text")));
    REQUIRE( stc->replace_next("less text", ""));
    REQUIRE(!stc->replace_next());
    REQUIRE(!stc->find_next(std::string("less text")));
    REQUIRE( stc->get_find_string() != "less text");
    REQUIRE( stc->replace_all("%", "percent") == 0);
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
    REQUIRE( lexer.set("cpp", true));
    REQUIRE(!lexer.set("xyz"));
    REQUIRE( stc->get_lexer().set(lexer));
  }

  SUBCASE("binary")
  {
    // do the same test as with wex::file in base for a binary file
    REQUIRE(stc->open(wex::test::get_path("test.bin")));
    REQUIRE(stc->data().flags() == 0);
    const auto& buffer = stc->GetTextRaw();
    REQUIRE(buffer.length() == 40);
  }
  
  SUBCASE("contents_changed")
  {
    stc->SetText("added text");
    REQUIRE( stc->GetText().Contains("added text"));
    REQUIRE( stc->get_file().get_contents_changed());
    stc->get_file().reset_contents_changed();
    REQUIRE(!stc->get_file().get_contents_changed());

    stc->AppendText("more text");
    REQUIRE( stc->GetText() != "hello stc");
  }
  
  SUBCASE("marker")
  {
    REQUIRE(stc->marker_delete_all_change());
  }

  SUBCASE("margin")
  {
    REQUIRE(stc->get_margin_text_click() == -1);

    stc->show_line_numbers(false);
    REQUIRE(!stc->is_shown_line_numbers());
    stc->show_line_numbers(true);
    REQUIRE( stc->is_shown_line_numbers());
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
    //  stc->filetype_menu();
    stc->fold();
    // FoldAll
    wex::config(_("stc.Auto fold")).set(3);
    stc->fold(true); 
    stc->Paste();
    //  stc->Print();
    stc->print_preview();
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

  SUBCASE("eol")
  {
    REQUIRE(!stc->eol().empty());
  }
    
  SUBCASE("position")
  {
    REQUIRE(!stc->position_restore());
    stc->position_save();
    REQUIRE( stc->position_restore());
  }
    
  SUBCASE("auto_indentation")
  {
    // first test auto indentation on next line
    wex::config(_("stc.Auto indent")).set(3);
    REQUIRE( wex::config(_("stc.Auto indent")).get(3) == 3);
    stc->set_text("  \n  line with indentation");
    stc->DocumentEnd();
    REQUIRE(!stc->auto_indentation('x'));
    REQUIRE( stc->GetText() == "  \n  line with indentation");
    REQUIRE( stc->GetLineCount() == 2);
    stc->SetEOLMode(wxSTC_EOL_CR);
    REQUIRE( stc->auto_indentation(stc->eol().front()));

    // the \n is not added, but indentation does
    REQUIRE( stc->GetText() == "  \n  line with indentation");
    REQUIRE( stc->GetLineCount() == 2);
    // test auto indentation for level change
    REQUIRE( stc->get_lexer().set("cpp"));
    stc->set_text("\nif ()\n{\n");
    stc->DocumentEnd();
    //  REQUIRE( stc->auto_indentation('\n'));
  }
  
  SUBCASE("link")
  {
    REQUIRE(!stc->link_open());
  }

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
    REQUIRE( stc.get_filename().string().find("test.h") != std::string::npos);
    REQUIRE( stc.open(wex::test::get_path("test.h")));
    REQUIRE(!stc.open("XXX"));
  }

  SUBCASE("omplete")
  {
    REQUIRE( stc->get_lexer().set("xml"));
    stc->get_vi().command("i<xxxx>");
    stc->get_vi().command("\x1b");
  }

  SUBCASE("popup")
  {
    REQUIRE( stc->get_lexer().set("cpp"));
  }
}