////////////////////////////////////////////////////////////////////////////////
// Name:      handle-show-message.cpp
// Purpose:   Implementation of JSON-RPC 2.0 protocol handler
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

bool json_rpc::handle_show_message(const json_rpc_message& notification) const
{
  auto* msg = new show_message_item(
    notification.params,
    notification.method == "window/showMessage");

  if (!msg->message.empty())
  {
    queue_event(m_event_handler, std::string(), ID_LSP_SHOW_MESSAGE, msg);

    return true;
  }

  delete msg;

  return false;
}
} // namespace lsp
} // namespace wex
