////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/vcs-command.h>

TEST_SUITE_BEGIN("wex::vcs");

TEST_CASE("wex::vcs_command")
{
  pugi::xml_document doc;
  doc.load_string("<command> a&dd </command>");
  const wex::vcs_command add(doc.document_element());
  doc.load_string("<command> blame </command>");
  const wex::vcs_command blame(doc.document_element());
  doc.load_string("<command> checkou&t </command>");
  const wex::vcs_command co(doc.document_element());
  doc.load_string("<command type=\"main\"> commit </command>");
  const wex::vcs_command commit(doc.document_element());
  doc.load_string(
    "<command type=\"popup\" submenu=\"submenu\"> diff </command>");
  const wex::vcs_command diff(doc.document_element());
  doc.load_string("<command type=\"main\"> log </command>");
  const wex::vcs_command log(doc.document_element());
  doc.load_string("<command subcommand=\"m&e\"> h&elp </command>");
  const wex::vcs_command help(doc.document_element());
  doc.load_string("<command> update </command>");
  const wex::vcs_command none;

  REQUIRE(add.get_command() == "add");
  REQUIRE(
    add.get_command(wex::menu_command::include_t()
                      .set(wex::menu_command::INCLUDE_SUBCOMMAND)
                      .set(wex::menu_command::INCLUDE_ACCELL)) == "a&dd");

  REQUIRE(add.is_add());
  REQUIRE(blame.is_blame());
  REQUIRE(co.is_checkout());
  REQUIRE(commit.is_commit());
  REQUIRE(diff.is_diff());
  REQUIRE(help.is_help());
  REQUIRE(log.is_history());
  REQUIRE(blame.is_open());
  REQUIRE(!help.ask_flags());
  REQUIRE(help.use_subcommand());

  REQUIRE(add.submenu().empty());
  REQUIRE(diff.submenu() == "submenu");
  REQUIRE(help.submenu() == "m&e");
}

TEST_SUITE_END();
