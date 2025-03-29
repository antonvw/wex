////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/util.h>
#include <wex/test/test.h>

TEST_CASE("wex::factory::utils")
{
  SECTION("node_properties")
  {
    std::vector<wex::property> properties;
    pugi::xml_document         doc;

    REQUIRE(doc.load_string("<properties>"
                            "  <property name = \"fold.comment\">2</property>"
                            "</properties>"));
    auto node = doc.document_element();

    wex::node_properties(&node, properties);

    REQUIRE(properties.size() == 1);
  }

  SECTION("node_styles")
  {
    std::vector<wex::style> styles;
    pugi::xml_document      doc;

    REQUIRE(doc.load_string("<styles>"
                            "  <style no = \"2\">string</style>"
                            "</styles>"));

    auto node = doc.document_element();

    wex::node_styles(&node, "cpp", styles);

    REQUIRE(styles.size() == 1);
  }
}
