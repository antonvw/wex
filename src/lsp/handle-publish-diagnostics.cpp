////////////////////////////////////////////////////////////////////////////////
// Name:      handle-publish-diagnostics.h
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
    const wex::diagnostic_item diag(diag_json.as_object());

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
