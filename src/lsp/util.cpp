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

  wxCommandEvent event(wxEVT_MENU, id);
  event.SetString(uri); // Pass the URI to the event handler
  event.SetClientData(data);
  wxPostEvent(handler, event); // Notify UI to update diagnostics display
}
} // namespace lsp
} // namespace wex
