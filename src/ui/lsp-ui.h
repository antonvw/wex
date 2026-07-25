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
/// Returns a path from a uri, with any percent‐escapes in the string decoded.
path make_path_skip_uri(const std::string& uri);

/// Sets lsp completions.
void set_lsp_completions(
  /// the stc to set completions on
  syntax::stc* stc,
  /// the completions to set
  const completions_t* completions,
  /// the frame to get trigger characters from
  wex::frame* frame);

/// Sets lsp definition or implementation.
void set_lsp_definition_or_implementation(
  /// the frame to open files in
  wex::frame* frame,
  /// the definitions or implementations to set
  const definition_or_implementation_t* definitions);

/// Sets lsp diagnostics.
void set_lsp_diagnostics(
  /// the stc to set diagnostics on
  syntax::stc* stc,
  /// the diagnostics to set
  const diagnostics_t* diagnostics);

/// Sets lsp hover.
void set_lsp_hover(
  /// the stc to set hover on
  syntax::stc* stc,
  /// the hover to set
  const hover_t* hover);

/// Sets lsp on type formatting.
void set_lsp_on_type(
  /// the stc to set on type formatting on
  syntax::stc* stc,
  /// the on type formatting items to set
  const on_type_formatting_item_t* items);

/// Sets lsp show message.
void set_lsp_show_message(
  /// the parent window
  wxWindow* parent,
  /// the show message item to set
  const show_message_item* item);
} // namespace wex
