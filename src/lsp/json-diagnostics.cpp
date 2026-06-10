////////////////////////////////////////////////////////////////////////////////
// Name:      notification-handler.h
// Purpose:   Declaration of JSON-RPC 2.0 protocol handler for diagnostics notifications
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/diagnostics.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
namespace lsp
{

bool json_rpc::handle_publish_diagnostics(const json_rpc_message& msg)
{
  const auto uri = msg.params["uri"].as_string();
  const auto diags_array = msg.params["diagnostics"].as_array();

  m_diagnostics.clear(uri); // Clear old diagnostics

  for (const auto& diag_json : diags_array)
  {
    wex::lsp::diagnostic diag;
    const auto range_json = diag_json["range"].as_object();
    diag.range.start_line = range_json["start"]["line"].as_int();
    diag.range.start_character = range_json["start"]["character"].as_int();
    diag.range.end_line = range_json["end"]["line"].as_int();
    diag.range.end_character = range_json["end"]["character"].as_int();
    diag.severity = static_cast<wex::lsp::severity_t>(diag_json["severity"].as_int());
    diag.code = diag_json["code"].as_string().c_str();
    diag.message = diag_json["message"].as_string().c_str();
    diag.source = diag_json["source"].as_string().c_str();

    // Store diagnostic
    m_diagnostics.add(uri, diag);
  }

  queue_event(m_event_handler, uri, ID_LSP_DIAGNOSTICS, get(uri)); // Notify UI to update diagnostics display   
  
  return true;
}

} // namespace lsp
} // namespace wex
