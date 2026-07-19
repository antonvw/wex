////////////////////////////////////////////////////////////////////////////////
// Name:      test-lsp.cpp
// Purpose:   Unit tests for LSP ui methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/lsp.h>

boost::json::object string_to_json(const std::string& text)
{
  auto parsed = boost::json::parse(text);
  return parsed.as_object();
}

TEST_CASE("wex::lsp")
{
  SECTION("position_item")
  {
    wex::position_item item;
    REQUIRE(item.line == 0);
    REQUIRE(item.character == 0);
  }

  SECTION("range_item")
  {
    wex::range_item item;
    REQUIRE(item.start.line == 0);
    REQUIRE(item.start.character == 0);
    REQUIRE(item.end.line == 0);
    REQUIRE(item.end.character == 0);
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

  SECTION("on_type_formatting")
  {
    const auto obj(string_to_json("{\
      \"range\": {\
        \"start\": { \"line\": 20, \"character\": 10 },\
        \"end\": { \"line\": 20, \"character\": 14 }\
      },\
      \"newText\": \"formatted\"\
      }"));

    wex::on_type_formatting_item item(obj);
    CAPTURE(item.new_text);
    REQUIRE(item.new_text == "formatted");
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

  SECTION("json_to_string") {}

  SECTION("range_from_json")
  {
    const auto obj(string_to_json("{\"range\":{\
         \"end\":{\"character\":30,\"line\":1153},\
         \"start\":{\"character\":19,\"line\":1151}}}"));

    wex::range_item range;

    REQUIRE(wex::range_from_json(obj, range));
    REQUIRE(range.start.line == 1151);
    REQUIRE(range.start.character == 19);
    REQUIRE(range.end.line == 1153);
    REQUIRE(range.end.character == 30);
  }
}
