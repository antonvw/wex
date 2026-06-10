////////////////////////////////////////////////////////////////////////////////
// Name:      frame-bind-lsp.cpp
// Purpose:   Implementation of frame::bind_lsp.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/syntax/stc.h>
#include <wex/ui/defs.h>
#include <wex/ui/frame.h>
#include <wex/ui/lsp.h>

namespace wex
{
  void set_lsp_diagnostics(syntax::stc* stc, diagnostics_t* diagnostics)
  {
    if (diagnostics != nullptr)
    {
      stc->indicator_clear_range(0, stc->GetTextLength());

      for (const auto& diag : *diagnostics)
      {
        stc->set_indicator(indicator(wex::data::stc().indicator_no()), 
          stc->PositionFromLine(diag.range.start_line), 
          stc->GetLineEndPosition(diag.range.end_line));
      }
    }
  }

  void set_lsp_hover(syntax::stc* stc, hover_t* hover)
  {
    if (hover != nullptr)
    {
      stc->indicator_clear_range(0, stc->GetTextLength());
      stc->set_indicator(
        indicator(wex::data::stc().indicator_no(), 
        stc->PositionFromLine(hover->line), 
        stc->GetLineEndPosition(hover->line)));
    }
  }
}

void wex::frame::bind_lsp()
{
  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        const auto& uri = after(event.GetString(), "file://");
        
        if (auto* stc = open_file(uri); stc != nullptr)
        {
          auto* diagnostics = std::dynamic_cast<diagnostics_t*>(event.GetClientData());
          set_lsp_diagnostics(stc, diagnostics);
        }
      },
      ID_LSP_DIAGNOSTICS}});

  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        const auto& uri = after(event.GetString(), "file://");
        
        if (auto* stc = open_file(uri); stc != nullptr)
        {
          auto* hover = std::dynamic_cast<hover_t*>(event.GetClientData());
          set_lsp_hover(stc, hover);
        }
      },
      ID_LSP_HOVER}});
}
