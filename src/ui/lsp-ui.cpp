////////////////////////////////////////////////////////////////////////////////
// Name:      lsp-ui.cpp
// Purpose:   Implementation of ui lsp methods.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <ranges>

#include <boost/algorithm/string.hpp>
#include <boost/url.hpp>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/syntax/indicator.h>
#include <wx/infobar.h>

#include "lsp-ui.h"

namespace wex
{
path make_path_skip_uri(const std::string& uri)
{
  const boost::urls::url u(uri);
  return path(u.path());
}

void set_lsp_completions(
  syntax::stc*         stc,
  const completions_t* completions,
  wex::frame*          frame)
{
  const auto         wsp = stc->WordStartPosition(stc->GetCurrentPos(), true);
  const std::string& filter(stc->GetTextRange(wsp, stc->GetCurrentPos()));

  if (!filter.empty() || !frame->lsp_clients_trigger(stc).empty())
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
  wex::frame*                           frame,
  const definition_or_implementation_t* definitions)
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

void set_lsp_diagnostics(syntax::stc* stc, const diagnostics_t* diagnostics)
{
  stc->AnnotationSetVisible(wxSTC_ANNOTATION_HIDDEN);
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

void set_lsp_hover(syntax::stc* stc, const hover_t* hover)
{
  if (stc->popup_menu_is_shown())
  {
    return;
  }

  std::string text(hover->contents.substr(1, hover->contents.size() - 2));
  boost::algorithm::replace_all(text, "\\n", "\n");

  if (stc->CallTipActive())
  {
    stc->CallTipCancel();
  }

  stc->CallTipShow(hover->pos.to_pos(stc), text);
}

void set_lsp_on_type(syntax::stc* stc, const on_type_formatting_item_t* items)
{
  // the items should first be sorted, because the language server
  // may return them in any order
  on_type_formatting_item_t sorted_items(items->begin(), items->end());

  std::ranges::sort(
    sorted_items,
    [stc](const auto& a, const auto& b)
    {
      return a.range.start.to_pos(stc) < b.range.start.to_pos(stc);
    });

  int last_pos = wxSTC_INVALID_POSITION;

  // apply the sorted items to the stc, and in reverse order to avoid messing
  // up the positions of the remaining items
  for (const auto& item : sorted_items | std::views::reverse)
  {
    log::trace("set_lsp_on_type") << item.log();
    item.replace_target(stc);
    last_pos =
      std::max(stc->GetCurrentPos() + (int)item.new_text.size(), last_pos);
  }

  if (last_pos != wxSTC_INVALID_POSITION)
  {
    stc->SetCurrentPos(last_pos);
    stc->SelectNone();
  }
}

void set_lsp_show_message(wxWindow* parent, const show_message_item* item)
{
  if (!item->is_show)
  {
    switch (item->type)
    {
      case show_message_item::DEBUG:
        log::debug(item->message);
        break;
      case show_message_item::ERRORS:
        log(item->message);
        break;
      case show_message_item::INFO:
        log::info(item->message);
        break;
      case show_message_item::LOG:
        log::trace(item->message);
        break;
      case show_message_item::WARNING:
        log::warning(item->message);
        break;
      default:
        log("unhandled show_message type") << static_cast<int>(item->type);
    }
  }
  else
  {
    auto* info = new wxInfoBar(parent);
    info->ShowMessage(item->message);
  }
}
} // namespace wex
