////////////////////////////////////////////////////////////////////////////////
// Name:      frame-lsp.h
// Purpose:   Implementation of frame ui lsp methods.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/lsp.h>

namespace wex
{
/// Returns a path from a uri, skipping the scheme and origin.
path make_path_skip_uri(const std::string& uri);

/// Sets lsp completions.
void set_lsp_completions(
  syntax::stc*         stc,
  const completions_t* completions,
  wex::frame*          frame);

/// Sets lsp definition or implementation.
void set_lsp_definition_or_implementation(
  wex::frame*                           frame,
  const definition_or_implementation_t* definitions);

/// Sets lsp diagnostics.
void set_lsp_diagnostics(syntax::stc* stc, const diagnostics_t* diagnostics);

/// Sets lsp hover.
void set_lsp_hover(syntax::stc* stc, const hover_t* hover);

/// Sets lsp on type formatting.
void set_lsp_on_type(syntax::stc* stc, const on_type_formatting_item_t* items);

/// Sets lsp show message.
void set_lsp_show_message(wxWindow* parent, const show_message_item* item);
} // namespace wex
