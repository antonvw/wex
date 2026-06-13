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

#include <wx/event.h>

#include <wex/lsp/diagnostics.h>

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
  /// Default constructor, provide event handler.
  json_rpc(wxEvtHandler* eh = nullptr);

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
    /// RPC method name
    const std::string& method,
    /// method parameters
    const boost::json::object& params = boost::json::object());

  /// Encodes a JSON-RPC request to string.
  /// Returns encoded message string.
  std::string encode_request(
    /// RPC method name
    const std::string& method,
    /// method parameters
    const boost::json::object& params = boost::json::object());

  /// Encodes a JSON-RPC response to string.
  /// Returns encoded message string.
  std::string encode_response(
    /// request ID
    int id,
    /// result
    const boost::json::object& result);

  /// Handles an incoming response.
  /// Returns false if msg is not handled.
  bool handle_response(const json_rpc_message& msg);

  /// Returns the JSON-RPC fixed content field header.
  std::string header_part_content_field() const;

  /// Registers a response handler for last request ID.
  void register_handler(
    /// callback to invoke when response arrives
    response_handler handler);

private:
  /// Handles an incoming diagnostics notification.
  /// Returns false if msg is not handled.
  bool handle_publish_diagnostics(const json_rpc_message& notification);

  std::string make_output(const boost::json::object& obj);

  diagnostics m_diagnostics;

  int m_id{1};

  std::unordered_map<int, response_handler> m_handlers;

  wxEvtHandler* m_event_handler;
};

} // namespace lsp
} // namespace wex
