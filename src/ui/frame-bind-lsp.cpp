////////////////////////////////////////////////////////////////////////////////
// Name:      frame-bind-lsp.cpp
// Purpose:   Implementation of frame::bind_lsp.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/syntax/indicator.h>
#include <wex/syntax/stc.h>
#include <wex/ui/defs.h>
#include <wex/ui/frame.h>
#include <wex/ui/lsp.h>

namespace wex
{
path make_path_skip_uri(const std::string& uri)
{
  return path(find_after(uri, "file://"));
}

void set_lsp_completions(syntax::stc* stc, completions_t* completions)
{
  const char separator = '~';
  std::string auto_complete_text;

  for (const auto& comp : completions->elements)
  {
    auto_complete_text += comp.label + separator;
  }

  stc->AutoCompSetSeparator(separator);
  stc->AutoCompShow(auto_complete_text.length() - 1, auto_complete_text);
  stc->AutoCompSetSeparator(' ');
}

void set_lsp_definition_or_implementation(
  wex::frame*                     frame,
  definition_or_implementation_t* definitions)
{
  for (const auto& def : *definitions)
  {
    data::control control;
    control.line(def.range.start_line + 1);
    control.col(def.range.start_character + 1);
    control.end_line(def.range.end_line + 1);
    control.end_col(def.range.end_character + 1);
    data::stc data(control);

    frame->open_file(make_path_skip_uri(def.uri), data);
  }

  delete definitions;
}

void set_lsp_diagnostics(syntax::stc* stc, diagnostics_t* diagnostics)
{
  stc->IndicatorClearRange(0, stc->GetTextLength());
  stc->AnnotationClearAll();

  for (const auto& diag : *diagnostics)
  {
    stc->set_indicator(
      indicator(wex::data::IND_ERROR),
      stc->PositionFromLine(diag.range.start_line),
      stc->GetLineEndPosition(diag.range.end_line));

    stc->SetAnnotationText(
      diag.range.start_line,
      diag.message + " (" + std::to_string(static_cast<int>(diag.severity)) +
        ")");
  }
}

void set_lsp_hover(syntax::stc* stc, hover_t* hover)
{
  stc->CallTipShow(stc->PositionFromLine(hover->line), hover->contents);
}
} // namespace wex

void wex::frame::bind_lsp()
{
  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        auto* completions = (completions_t*)event.GetClientData();

        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          set_lsp_completions(dynamic_cast<syntax::stc*>(stc), completions);
        }

        delete completions;
      },
      ID_LSP_CODE_COMPLETION},

     {[=, this](const wxCommandEvent& event)
      {
        auto* definition =
          (definition_or_implementation_t*)event.GetClientData();
        set_lsp_definition_or_implementation(this, definition);
      },
      ID_LSP_DEFINITION},

     {[=, this](const wxCommandEvent& event)
      {
        auto* diagnostics = (diagnostics_t*)event.GetClientData();

        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          set_lsp_diagnostics(dynamic_cast<syntax::stc*>(stc), diagnostics);
        }

        delete diagnostics;
      },
      ID_LSP_DIAGNOSTICS},

     {[=, this](const wxCommandEvent& event)
      {
        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          auto* hover = (hover_t*)event.GetClientData();
          set_lsp_hover(dynamic_cast<syntax::stc*>(stc), hover);
          delete hover;
        }
      },
      ID_LSP_HOVER},

     {[=, this](const wxCommandEvent& event)
      {
        auto* definition =
          (definition_or_implementation_t*)event.GetClientData();
        set_lsp_definition_or_implementation(this, definition);
      },
      ID_LSP_IMPLEMENTATION}});
}
