////////////////////////////////////////////////////////////////////////////////
// Name:      notification-handler.h
// Purpose:   Declaration of class wex::lsp::notification-handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/lsp/diagnostics.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
namespace lsp
{
/// Listens for diagnostics notifications from server, handles them and notifies the UI.
class notification_handler
{
public:
  /// Constructor, starts listening for notifications.
  notification_handler(const std::unique_ptr<boost::process::popen>& process);

private:
  void handle_publish_diagnostics(const json_rpc_message& notification);

  const std::unique_ptr<boost::process::popen>& m_process;
  diagnostics m_diagnostics; ///< Manages diagnostics for documents.
}
}
