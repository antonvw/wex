////////////////////////////////////////////////////////////////////////////////
// Name:      json-rpc-test.cpp
// Purpose:   Unit tests for JSON-RPC 2.0 protocol handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/json-rpc.h>
#include <wex/test/test.h>

TEST_CASE("wex::lsp::json_rpc")
{
  wex::lsp::json_rpc rpc;

  SECTION("encode_request")
  {
    boost::json::object params;
    params["processId"] = nullptr;
    params["rootPath"]  = "/home/user/project";

    std::string encoded = rpc.encode_request("initialize", params);

    REQUIRE(encoded.find("Content-Length:") != std::string::npos);
    REQUIRE(encoded.find("initialize") != std::string::npos);
    REQUIRE(encoded.find("2.0") != std::string::npos);
  }

  SECTION("encode_notification")
  {
    boost::json::object params;
    params["uri"] = "file:///test.cpp";

    std::string encoded =
      rpc.encode_notification("textDocument/didOpen", params);

    REQUIRE(encoded.find("Content-Length:") != std::string::npos);
    REQUIRE(encoded.find("textDocument/didOpen") != std::string::npos);
    REQUIRE(encoded.find("id") != std::string::npos);
  }

  SECTION("encode_response")
  {
    boost::json::object result;
    result["capabilities"] = boost::json::object();

    std::string encoded = rpc.encode_response(1, result);

    REQUIRE(encoded.find("Content-Length:") != std::string::npos);
    REQUIRE(encoded.find("capabilities") != std::string::npos);
    REQUIRE(encoded.find("\"id\":1") != std::string::npos);
  }

  SECTION("encode_error")
  {
    std::string encoded = rpc.encode_error(1, -32600, "Invalid Request");

    REQUIRE(encoded.find("Content-Length:") != std::string::npos);
    REQUIRE(encoded.find("Invalid Request") != std::string::npos);
    REQUIRE(encoded.find("error") != std::string::npos);
  }

  SECTION("decode_request")
  {
    std::string message =
      "Content-Length: 63\r\n\r\n"
      "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"initialize\",\"params\":{}}";

    wex::lsp::json_rpc_message msg = rpc.decode(message);

    REQUIRE(msg.id == 1);
    REQUIRE(msg.method == "initialize");
  }

  SECTION("decode_response")
  {
    std::string message =
      "Content-Length: 58\r\n\r\n"
      "{\"jsonrpc\":\"2.0\",\"id\":1,\"result\":{\"capabilities\":{}}}";

    wex::lsp::json_rpc_message msg = rpc.decode(message);

    REQUIRE(msg.id == 1);
    REQUIRE(msg.is_error == false);
  }

  SECTION("decode_error")
  {
    std::string message = "Content-Length: 76\r\n\r\n"
                          "{\"jsonrpc\":\"2.0\",\"id\":1,\"error\":{\"code\":-"
                          "32600,\"message\":\"Invalid\"}}";

    wex::lsp::json_rpc_message msg = rpc.decode(message);

    REQUIRE(msg.is_error == true);
    REQUIRE(msg.id == 1);
  }

  SECTION("register_and_handle_response")
  {
    bool                       handled = false;
    wex::lsp::json_rpc_message received_msg;

    wex::lsp::response_handler hdl(
      [&](const wex::lsp::json_rpc_message& msg)
      {
        handled      = true;
        received_msg = msg;
      });

    rpc.register_handler(hdl);

    wex::lsp::json_rpc_message msg;
    msg.id = 1;

    rpc.handle_response(msg);

    REQUIRE(handled == true);
    REQUIRE(received_msg.id == 1);
  }
}
