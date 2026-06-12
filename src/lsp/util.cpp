////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.h
// Purpose:   Declaration of main LSP components
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/lsp/util.h>
#include <wx/event.h>

namespace wex
{
namespace lsp
{
void queue_event(
  wxEvtHandler*      handler,
  const std::string& uri,
  int                id,
  const void*        data)
{
  if (handler == nullptr)
  {
    return;
  }

  auto* event = new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, id);
  event->SetString(uri); // Pass the URI to the event handler
  event->SetClientData(data);
  wxQueueEvent(handler, event); // Notify UI to update diagnostics display
}
} // namespace lsp
} // namespace wex
