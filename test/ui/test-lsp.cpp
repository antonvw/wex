////////////////////////////////////////////////////////////////////////////////
// Name:      test-lsp.cpp
// Purpose:   Unit tests for LSP ui methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/lsp.h>

#include "test.h"

boost::json::object string_to_json(const std::string& text)
{
  auto parsed = boost::json::parse(text);
  return parsed.as_object();
}

TEST_CASE("wex::lsp")
{
  auto* stc = new wex::test::ui_stc();

  SECTION("position_item")
  {
    wex::position_item item;
    REQUIRE(item.line == 0);
    REQUIRE(item.character == 0);
    REQUIRE(item.to_pos(stc) == 0);
  }

  SECTION("range_item")
  {
    wex::range_item item;
    REQUIRE(item.start.line == 0);
    REQUIRE(item.start.character == 0);
    REQUIRE(item.end.line == 0);
    REQUIRE(item.end.character == 0);

    item.set_target(stc);
    REQUIRE(stc->GetTargetStart() == 0);
    REQUIRE(stc->GetTargetEnd() == 0);

    const auto obj(string_to_json("{\"range\":{\
         \"end\":{\"character\":30,\"line\":1153},\
         \"start\":{\"character\":19,\"line\":1151}}}"));

    REQUIRE(item.set(obj));
    REQUIRE(item.start.line == 1151);
    REQUIRE(item.start.character == 19);
    REQUIRE(item.end.line == 1153);
    REQUIRE(item.end.character == 30);
  }

  SECTION("completion_item_element")
  {
    const auto obj(string_to_json("{\"range\":{\
         \"end\":{\"character\":30,\"line\":1153},\
         \"start\":{\"character\":19,\"line\":1151}}}"));

    wex::completion_item_element item(obj);
    REQUIRE(item.label.empty());
    REQUIRE(item.kind == 0);
    REQUIRE(item.detail.empty());
    REQUIRE(item.documentation.empty());
  }

  SECTION("completion_item")
  {
    wex::completion_item item;
    REQUIRE(item.pos.line == 0);
    REQUIRE(item.pos.character == 0);
  }

  SECTION("definition_or_implementation_item")
  {
    const auto obj(string_to_json("{\
       \"uri\": \"file:///project/src/utils.ts\",\
       \"range\": {\
         \"start\": { \"line\": 10, \"character\": 2 },\
         \"end\": { \"line\": 11, \"character\": 5 }\
       }}"));

    wex::definition_or_implementation_item item(obj);
    REQUIRE(item.uri == "file:///project/src/utils.ts");
    REQUIRE(item.range.start.line == 10);
    REQUIRE(item.range.start.character == 2);
    REQUIRE(item.range.end.line == 11);
    REQUIRE(item.range.end.character == 5);
  }

  SECTION("diagnostic_item")
  {
    const auto obj(string_to_json("{\
      \"range\": {\
        \"start\": { \"line\": 3, \"character\": 10 },\
        \"end\":   { \"line\": 4, \"character\": 15 }\
      },\
      \"severity\": 1,\
      \"code\": \"TS2345\",\
      \"source\": \"typescript\",\
      \"message\": \"Argument of type 'string' is not assignable to parameter of type 'number'.\"\
    }"));

    wex::diagnostic_item item(obj);
    REQUIRE(item.range.start.line == 3);
    REQUIRE(item.range.start.character == 10);
    REQUIRE(item.range.end.line == 4);
    REQUIRE(item.range.end.character == 15);
  }

  SECTION("hover_item")
  {
    const auto obj(string_to_json("{\
      \"contents\": {\
        \"kind\": \"markdown\",\
        \"value\": \"### add(a, b)\\n\\nAdds two numbers and returns the result\"\
      },\
      \"range\": {\
        \"start\": { \"line\": 10, \"character\": 2 },\
        \"end\":   { \"line\": 10, \"character\": 5 }\
      }\
    }"));

    wex::hover_item item(obj);
    CAPTURE(item.contents);
    REQUIRE(item.contents.starts_with("\"### add"));
    REQUIRE(item.kind == "markdown");
  }

  SECTION("json_to_string") {}

  SECTION("on_type_formatting")
  {
    const auto obj(string_to_json("{\
      \"range\": {\
        \"start\": { \"line\": 0, \"character\": 10 },\
        \"end\": { \"line\": 0, \"character\": 14 }\
      },\
      \"newText\": \"formatted\"\
      }"));

    stc->set_text("This is a test string for on-type formatting");

    wex::on_type_formatting_item item(obj);
    CAPTURE(item.new_text);
    REQUIRE(item.range.start.to_pos(stc) == 10);
    REQUIRE(item.range.end.to_pos(stc) == 14);
    REQUIRE(item.new_text == "formatted");
    REQUIRE(item.replace_target(stc) == 5);
    REQUIRE(
      stc->GetText() == "This is a formatted string for on-type formatting");
  }

  SECTION("show_message_item")
  {
    const auto obj(string_to_json("{\
      \"type\": 1,\
      \"message\": \"Build succeeded in 1.24s\"\
    }"));

    wex::show_message_item item(obj);
    REQUIRE(item.type == 1);
    REQUIRE(item.is_show);
    CAPTURE(item.message);
    REQUIRE(item.message.starts_with("Build succ"));
  }
}
