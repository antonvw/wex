////////////////////////////////////////////////////////////////////////////////
// Name:      frame-bind-lsp.cpp
// Purpose:   Implementation of frame::bind_lsp.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/url.hpp>
#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/factory/util.h>
#include <wex/syntax/indicator.h>
#include <wex/syntax/stc.h>
#include <wex/ui/defs.h>
#include <wex/ui/frame.h>
#include <wex/ui/lsp.h>

namespace wex
{
path make_path_skip_uri(const std::string& uri)
{
  boost::urls::url u(uri);
  u.remove_scheme().remove_origin();
  return path(u.normalize_path().buffer());
}

void set_lsp_completions(syntax::stc* stc, completions_t* completions)
{
  const auto         wsp = stc->WordStartPosition(stc->GetCurrentPos(), true);
  const std::string& filter(stc->GetTextRange(wsp, stc->GetCurrentPos()));

  if (!filter.empty() || !get_trigger(stc).empty())
  {
    const char  separator = 3;
    std::string auto_complete_text;

    for (const auto& comp : completions->elements)
    {
      if (comp.label.starts_with(filter))
      {
        auto_complete_text += comp.label + separator;
      }
    }

    if (!auto_complete_text.empty())
    {
      const auto old(stc->AutoCompGetSeparator());
      stc->AutoCompSetSeparator(separator);
      stc->AutoCompShow(stc->GetCurrentPos() - wsp, auto_complete_text);
      stc->AutoCompSetSeparator(old);
    }
  }
}

void set_lsp_definition_or_implementation(
  wex::frame*                     frame,
  definition_or_implementation_t* definitions)
{
  for (const auto& def : *definitions)
  {
    data::control control;
    control.line(def.range.start.line + 1);
    control.col(def.range.start.character + 1);
    control.end_line(def.range.end.line + 1);
    control.end_col(def.range.end.character + 1);
    data::stc data(control);

    frame->open_file(make_path_skip_uri(def.uri), data);
  }

  delete definitions;
}

void set_lsp_diagnostics(syntax::stc* stc, diagnostics_t* diagnostics)
{
  stc->SetIndicatorCurrent(wex::data::stc::IND_ERR);
  stc->IndicatorClearRange(0, stc->GetTextLength());
  stc->AnnotationClearAll();

  for (const auto& diag : *diagnostics)
  {
    stc->set_indicator(
      indicator(wex::data::stc::IND_ERR),
      stc->PositionFromLine(diag.range.start.line),
      stc->GetLineEndPosition(diag.range.end.line));

    stc->AnnotationSetText(
      diag.range.start.line,
      lexer().align_text(
        diag.message + " (" + std::to_string(static_cast<int>(diag.severity)) +
        ")"));
  }
}

void set_lsp_hover(syntax::stc* stc, hover_t* hover)
{
  std::string text(hover->contents.substr(1, hover->contents.size() - 2));
  boost::algorithm::replace_all(text, "\\n", "\n");

  if (stc->CallTipActive())
  {
    stc->CallTipCancel();
  }

  stc->CallTipShow(
    stc->PositionFromLine(hover->pos.line) + hover->pos.character,
    text);
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
        if (
          auto* definition =
            (definition_or_implementation_t*)event.GetClientData();
          definition != nullptr)
        {
          set_lsp_definition_or_implementation(this, definition);
        }
      },
      ID_LSP_DEFINITION},

     {[=, this](const wxCommandEvent& event)
      {
        auto* diagnostics = (diagnostics_t*)event.GetClientData();

        if (const path cur(make_path_skip_uri(event.GetString())); is_open(cur))
        {
          if (auto* stc = open_file(cur); stc != nullptr)
          {
            set_lsp_diagnostics(dynamic_cast<syntax::stc*>(stc), diagnostics);
          }
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
        if (
          auto* definition =
            (definition_or_implementation_t*)event.GetClientData();
          definition != nullptr)
        {
          set_lsp_definition_or_implementation(this, definition);
        }
      },
      ID_LSP_IMPLEMENTATION}});
}
