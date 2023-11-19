////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/menu-command.h>
#include <wex/test/test.h>

TEST_CASE("wex::menu_command")
{
  pugi::xml_document doc;

  SUBCASE("Default constructor")
  {
    const wex::menu_command none;
    REQUIRE(none.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(none.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(!none.type().test(wex::menu_command::IS_VISUAL));
  }

  SUBCASE("control")
  {
    doc.load_string("<command control=\"x\"> a&dd </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.get_command() == "add");
    REQUIRE(cmd.control() == "x");
    REQUIRE(
      cmd.get_command(wex::menu_command::include_t()
                        .set(wex::menu_command::INCLUDE_SUBCOMMAND)
                        .set(wex::menu_command::INCLUDE_ACCELL)) == "a&dd");
    REQUIRE(cmd.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(cmd.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(cmd.submenu().empty());
  }

  SUBCASE("ellipses")
  {
    doc.load_string("<command type=\"ellipses\"> ask </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.type().test(wex::menu_command::ELLIPSES));
    REQUIRE(!cmd.type().test(wex::menu_command::IS_ASKED));
  }

  SUBCASE("ellipses-is-asked")
  {
    doc.load_string("<command type=\"ellipses-is-asked\"> ask </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.type().test(wex::menu_command::ELLIPSES));
    REQUIRE(cmd.type().test(wex::menu_command::IS_ASKED));
  }

  SUBCASE("flags")
  {
    doc.load_string("<command flags=\"hello\"> world </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.flags() == "hello");
  }

  SUBCASE("menu")
  {
    doc.load_string("<command menu=\"blame line\"> blame </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(cmd.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(cmd.text() == "blame line");
    REQUIRE(cmd.get_command() == "blame");
  }

  SUBCASE("subcommand")
  {
    doc.load_string("<command subcommand=\"m&e\"> h&elp </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.get_command() == "help me");
    REQUIRE(
      cmd.get_command(wex::menu_command::include_t()
                        .set(wex::menu_command::INCLUDE_SUBCOMMAND)
                        .set(wex::menu_command::INCLUDE_ACCELL)) ==
      "h&elp m&e");
    REQUIRE(
      cmd.get_command(wex::menu_command::include_t().set(
        wex::menu_command::INCLUDE_ACCELL)) == "h&elp");
    REQUIRE(cmd.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(cmd.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(!cmd.ask_flags());
    REQUIRE(cmd.is_help());
    REQUIRE(cmd.use_subcommand());
    REQUIRE(cmd.submenu() == "m&e");
  }

  SUBCASE("type")
  {
    doc.load_string(
      "<command type=\"main is-selected is-visual\"> commit </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(cmd.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(!cmd.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(cmd.type().test(wex::menu_command::IS_SELECTED));
    REQUIRE(cmd.type().test(wex::menu_command::IS_VISUAL));
  }

  SUBCASE("type and submenu")
  {
    doc.load_string(
      "<command type=\"popup\" submenu=\"submenu\"> diff </command>");
    const wex::menu_command cmd(doc.document_element());
    REQUIRE(!cmd.type().test(wex::menu_command::IS_MAIN));
    REQUIRE(cmd.type().test(wex::menu_command::IS_POPUP));
    REQUIRE(!cmd.type().test(wex::menu_command::IS_SELECTED));
    REQUIRE(cmd.submenu() == "submenu");
  }
}
