////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.h
// Purpose:   Declaration of main LSP components
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <wex/lsp/util.h>
#include <wx/event.h>

namespace wex
{
namespace lsp
{
bool range_from_json(const boost::json::object& obj, range_item& range)
{
  auto ro = obj.at("range");

  range.start_line      = ro.at("start").at("line").as_int64();
  range.start_character = ro.at("start").at("character").as_int64();
  range.end_line        = ro.at("end").at("line").as_int64();
  range.end_character   = ro.at("end").at("character").as_int64();

  return true;
}

void queue_event(
  wxEvtHandler*      handler,
  const std::string& uri,
  int                id,
  void*              data)
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
