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
  if (completions != nullptr)
  {
    stc->IndicatorClearRange(0, stc->GetTextLength());
    stc->set_indicator(
      indicator(wex::data::stc().indicator_no()),
      stc->PositionFromLine(completions->line),
      stc->GetLineEndPosition(completions->line));

    for (const auto& comp : completions->elements)
    {
      // Here you would typically display the completion items in a popup or
      // some UI element.
      // std::cout << "Completion: " << comp.label << " (Kind: " << comp.kind
      //          << ", Detail: " << comp.detail
      //          << ", Documentation: " << comp.documentation << ")\n";
    }

    delete completions;
  }
}

void set_lsp_definition(wex::frame* frame, definition_t* definitions)
{
  if (definitions != nullptr)
  {
    for (const auto& def : *definitions)
    {
      if (
        auto* stc = frame->open_file(make_path_skip_uri(def.uri));
        stc != nullptr)
      {
        stc->goto_line(def.range.start_line);
      }
    }

    delete definitions;
  }
}

void set_lsp_diagnostics(syntax::stc* stc, diagnostics_t* diagnostics)
{
  if (diagnostics != nullptr)
  {
    stc->IndicatorClearRange(0, stc->GetTextLength());

    for (const auto& diag : *diagnostics)
    {
      stc->set_indicator(
        indicator(wex::data::stc().indicator_no()),
        stc->PositionFromLine(diag.range.start_line),
        stc->GetLineEndPosition(diag.range.end_line));
    }

    delete diagnostics;
  }
}

void set_lsp_hover(syntax::stc* stc, hover_t* hover)
{
  if (hover != nullptr)
  {
    stc->IndicatorClearRange(0, stc->GetTextLength());
    stc->set_indicator(
      indicator(wex::data::stc().indicator_no()),
      stc->PositionFromLine(hover->line),
      stc->GetLineEndPosition(hover->line));

    delete hover;
  }
}
} // namespace wex

void wex::frame::bind_lsp()
{
  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          auto* completions = (completions_t*)event.GetClientData();
          set_lsp_completions(dynamic_cast<syntax::stc*>(stc), completions);
        }
      },
      ID_LSP_CODE_COMPLETION},

     {[=, this](const wxCommandEvent& event)
      {
        auto* definition = (definition_t*)event.GetClientData();
        set_lsp_definition(this, definition);
      },
      ID_LSP_DEFINITION},

     {[=, this](const wxCommandEvent& event)
      {
        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          auto* diagnostics = (diagnostics_t*)event.GetClientData();
          set_lsp_diagnostics(dynamic_cast<syntax::stc*>(stc), diagnostics);
        }
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
        }
      },
      ID_LSP_HOVER}});
}
