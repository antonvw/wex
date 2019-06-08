////////////////////////////////////////////////////////////////////////////////
// Name:      test-menu_command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/menucommand.h>
#include "test.h"

TEST_CASE("wex::menu_command")
{
  pugi::xml_document doc;
  
  doc.load_string("<command control=\"x\"> a&dd </command>");
  const wex::menu_command add(doc.document_element());
  doc.load_string("<command> blame </command>");
  const wex::menu_command blame(doc.document_element());
  doc.load_string("<command> checkou&t </command>");
  const wex::menu_command co(doc.document_element());
  doc.load_string("<command type=\"main\"> commit </command>");
  const wex::menu_command commit(doc.document_element());
  doc.load_string("<command type=\"popup\" submenu=\"submenu\"> diff </command>");
  const wex::menu_command diff(doc.document_element());
  doc.load_string("<command type=\"main\"> log </command>");
  const wex::menu_command log(doc.document_element());
  doc.load_string("<command subcommand=\"m&e\"> h&elp </command>");
  const wex::menu_command help(doc.document_element());
  doc.load_string("<command> update </command>");
  const wex::menu_command update(doc.document_element());
  const wex::menu_command none;

  REQUIRE(add.get_command() == "add");
  REQUIRE(add.control() == "x");
  REQUIRE(add.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_SUBCOMMAND).set(
    wex::menu_command::INCLUDE_ACCELL)) == "a&dd");
  
  REQUIRE(help.get_command() == "help me");
  REQUIRE(help.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_SUBCOMMAND).set(
    wex::menu_command::INCLUDE_ACCELL)) == "h&elp m&e");
  REQUIRE(help.get_command(wex::menu_command::include_t().set(
    wex::menu_command::INCLUDE_ACCELL)) == "h&elp");
  
  REQUIRE( add.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( add.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( blame.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( blame.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( commit.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( diff.type().test(wex::menu_command::IS_POPUP));
  REQUIRE( help.type().test(wex::menu_command::IS_MAIN));
  REQUIRE( help.type().test(wex::menu_command::IS_POPUP));

  REQUIRE(!help.ask_flags());
  REQUIRE( help.is_help());
  REQUIRE( help.use_subcommand());

  REQUIRE( add.submenu().empty());
  REQUIRE( diff.submenu() == "submenu");
  REQUIRE( help.submenu() == "m&e");

  REQUIRE(none.type().none());
}
