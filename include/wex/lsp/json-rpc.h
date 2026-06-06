////////////////////////////////////////////////////////////////////////////////
// Name:      json-rpc.h
// Purpose:   Declaration of JSON-RPC 2.0 protocol handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace wex
{
namespace lsp
{
/// Represents a JSON-RPC 2.0 message.
struct json_rpc_message
{
  std::string         jsonrpc{"2.0"};
  int                 id{-1}; // -1 for notifications
  std::string         method;
  boost::json::object params;
  boost::json::object result;
  boost::json::object error;
  bool                is_response{false};
  bool                is_error{false};
};

/// Callback for handling JSON-RPC responses.
using response_handler = std::function<void(const json_rpc_message&)>;

/// Handles JSON-RPC 2.0 protocol for LSP.
class json_rpc
{
public:
  /// Decodes a JSON-RPC message from string.
  /// Returns parsed message, or empty if parsing failed.
  json_rpc_message decode(
    /// Raw message data
    const std::string& data);

  /// Encodes a JSON-RPC error response to string.
  /// Return encoded message string.
  std::string encode_error(
    /// request ID
    int id,
    /// error code
    int code,
    /// error message
    const std::string& message);

  /// Encodes a JSON-RPC notification to string.
  /// Returns encoded message string.
  std::string encode_notification(
    /// the RPC method name
    const std::string& method,
    /// the method parameters
    const boost::json::object& params = boost::json::object());

  /// Encodes a JSON-RPC request to string.
  /// Returns encoded message string.
  std::string encode_request(
    /// the RPC method name
    const std::string& method,
    /// the method parameters
    const boost::json::object& params = boost::json::object(),
    /// request ID (will be auto-incremented)
    int id = -1);

  /// Encodes a JSON-RPC response to string.
  /// Returns encoded message string.
  std::string encode_response(
    /// Request ID
    int id,
    /// the result
    const boost::json::object& result);

  /// Handles an incoming response.
  /// Returns false if msg is not handled.
  bool handle_response(const json_rpc_message& msg);

  /// Returns the next request ID.
  int next_id() { return ++m_next_id; }

  /// Registers a response handler for a request ID.
  void register_handler(
    /// request ID
    int id,
    /// callback to invoke when response arrives
    response_handler handler);

private:
  int m_next_id{0};

  std::unordered_map<int, response_handler> m_handlers;
  static constexpr const char* CONTENT_LENGTH_HEADER = "Content-Length: ";
};

} // namespace lsp
} // namespace wex
