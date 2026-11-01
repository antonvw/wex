////////////////////////////////////////////////////////////////////////////////
// Name:      test-lsp-item.h
// Purpose:   Unit tests for LSP core methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>

boost::json::object string_to_json(const std::string& text)
{
  auto parsed = boost::json::parse(text);
  return parsed.as_object();
}

boost::json::object test_inactive_regions()
{
  const auto obj(string_to_json("{\
    \"textDocument\": {\
      \"uri\": \"file:///project/main.cpp\"\
    },\
    \"regions\": [\
      {\
        \"start\": { \"line\": 1, \"character\": 0 },\
        \"end\":   { \"line\": 3, \"character\": 0 }\
      }\
    ]\
    }"));

  return obj;
}
