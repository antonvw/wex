////////////////////////////////////////////////////////////////////////////////
// Name:      notification-handler.h
// Purpose:   Declaration of class wex::lsp::notification-handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/lsp/client.h>
#include <wex/lsp/diagnostics.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
namespace lsp
{
class client;

/// Listens for diagnostics notifications from server, handles them and notifies
/// the UI.
class notification_handler
{
public:
  /// Constructor, starts listening for notifications.
  notification_handler(client* c)
    : m_client(c)
  {
    listen();
  }

  /// Listens.
  void listen();

private:
  void handle_publish_diagnostics(json_rpc_message& notification);

  client*     m_client;
  diagnostics m_diagnostics; ///< Manages diagnostics for documents.
};

// implementation

inline void notification_handler::listen() {}

inline void
notification_handler::handle_publish_diagnostics(json_rpc_message& notification)
{
  const std::string uri = ""; // notification.params["uri"].as_string();
  const auto        diags_array = notification.params["diagnostics"].as_array();

  m_diagnostics.clear(uri); // Clear old diagnostics

  for (const auto& diag_json : diags_array)
  {
    wex::lsp::diagnostic diag;
    // Parse and store diagnostic
    m_diagnostics.add(uri, diag);
  }

  // Notify UI about diagnostics update
  // on_diagnostics_updated(uri);
}
} // namespace lsp
} // namespace wex
