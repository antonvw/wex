////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Declaration of utility functions for LSP support
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>
#include <string>

#include <wex/ui/lsp.h>

class wxEvtHandler;

namespace wex
{
namespace lsp
{
/// Language Server Protocol support for wex.
void queue_event(
  wxEvtHandler*      handler,
  const std::string& uri,
  int                id,
  void*              data);

/// Convert a json range object to a range item.
bool range_from_json(const boost::json::object& obj, range_item& range);
} // namespace lsp
} // namespace wex
