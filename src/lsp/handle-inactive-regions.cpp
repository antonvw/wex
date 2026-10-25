////////////////////////////////////////////////////////////////////////////////
// Name:      handle-inactive-regions.cpp
// Purpose:   Implementation of handle_inactive_regions
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

bool json_rpc::handle_inactive_regions(const json_rpc_message& notification)
{
  auto* msg = new inactive_regions_item(notification.params);

  if (!msg->ranges.empty())
  {
    queue_event(m_event_handler, std::string(), ID_LSP_INACTIVE_REGIONS, msg);

    return true;
  }

  delete msg;

  return false;
}
} // namespace lsp
} // namespace wex
