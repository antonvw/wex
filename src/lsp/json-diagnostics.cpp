////////////////////////////////////////////////////////////////////////////////
// Name:      json-diagnostics.h
// Purpose:   protocol handler for diagnostics notifications
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/json-rpc.h>
#include <wex/lsp/util.h>
#include <wex/ui/defs.h>

namespace wex
{
namespace lsp
{

bool json_rpc::handle_publish_diagnostics(const json_rpc_message& msg)
{
  const auto uri         = msg.params.at("uri").as_string();
  const auto diags_array = msg.params.at("diagnostics").as_array();

  m_diagnostics.clear(uri.data()); // Clear old diagnostics

  for (const auto& diag_json : diags_array)
  {
    wex::diagnostic_item diag;
    range_from_json(diag_json.as_object(), diag.range);
    diag.severity =
      static_cast<wex::severity_t>(diag_json.at("severity").as_int64());
    diag.code    = diag_json.at("code").as_string().c_str();
    diag.message = diag_json.at("message").as_string().c_str();
    diag.source  = diag_json.at("source").as_string().c_str();

    // Store diagnostic
    m_diagnostics.add(uri.data(), diag);
  }

  auto* obj = new diagnostics_t;
  *obj      = m_diagnostics.get(uri.data());

  queue_event(m_event_handler, uri.data(), ID_LSP_DIAGNOSTICS, obj);

  return true;
}

} // namespace lsp
} // namespace wex
