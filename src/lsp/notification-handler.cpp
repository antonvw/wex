////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <wex/core/log.h>
#include <wex/lsp/notification-handler.h>
#include <wex/syntax/lexers.h>

namespace wex
{
namespace lsp
{
notification_handler::notification_handler(const std::unique_ptr<boost::process::popen>& process)
  : m_process(process)
{
  // Start listening for notifications, e.g., by subscribing to a message bus
  // message_bus.subscribe("lsp/diagnostics", [this](const json_rpc_message& msg) {
  //     handle_publish_diagnostics(msg);
  // });
}

void notification_handler::handle_publish_diagnostics(const json_rpc_message& notification) 
{
  auto uri = notification.params["uri"].as_string();
  auto diags_array = notification.params["diagnostics"].as_array();
    
  m_diagnostics.clear(uri);  // Clear old diagnostics

  for (const auto& diag_json : diags_array)
  {
      wex::lsp::diagnostic diag;
      // Parse and store diagnostic
      m_diagnostics.add(uri, diag);
  }
    
  // Notify UI about diagnostics update
  //on_diagnostics_updated(uri);
}
}
}
