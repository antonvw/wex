////////////////////////////////////////////////////////////////////////////////
// Name:      lsp-frame-bind.cpp
// Purpose:   Implementation of frame::bind_lsp.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/bind.h>
#include <wex/ui/defs.h>

#include "lsp-ui.h"

void wex::frame::bind_lsp()
{
  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        auto* item = (completions_t*)event.GetClientData();

        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()));
          stc != nullptr)
        {
          set_lsp_completions(
            dynamic_cast<syntax::stc*>(stc),
            item,
            this);
        }

        delete item;
      },
      ID_LSP_CODE_COMPLETION},

     {[=, this](const wxCommandEvent& event)
      {
        if (
          auto* item = (definition_or_implementation_t*)event.GetClientData();
          item != nullptr)
        {
          set_lsp_definition_or_implementation(this, item);
        }
      },
      ID_LSP_DEFINITION},

     {[=, this](const wxCommandEvent& event)
      {
        auto* item = (diagnostics_t*)event.GetClientData();

        if (const path cur(make_path_skip_uri(event.GetString())); is_open(cur))
        {
          if (auto* stc = open_file(cur); stc != nullptr)
          {
            set_lsp_diagnostics(dynamic_cast<syntax::stc*>(stc), item);
          }
        }

        delete item;
      },
      ID_LSP_DIAGNOSTICS},

     {[=, this](const wxCommandEvent& event)
      {
        data::stc data;
        data.allow_change_page(false);

        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()), data);
          stc != nullptr)
        {
          auto* item = (on_type_formatting_item_t*)event.GetClientData();
          set_lsp_on_type(dynamic_cast<syntax::stc*>(stc), item);
          delete item;
        }
      },
      ID_LSP_FORMAT},

     {[=, this](const wxCommandEvent& event)
      {
        data::stc data;
        data.allow_change_page(false);

        if (
          auto* stc = open_file(make_path_skip_uri(event.GetString()), data);
          stc != nullptr)
        {
          auto* item = (hover_t*)event.GetClientData();
          set_lsp_hover(dynamic_cast<syntax::stc*>(stc), item);
          delete item;
        }
      },
      ID_LSP_HOVER},

     {[=, this](const wxCommandEvent& event)
      {
        if (
          auto* item = (definition_or_implementation_t*)event.GetClientData();
          item != nullptr)
        {
          set_lsp_definition_or_implementation(this, item);
        }
      },
      ID_LSP_IMPLEMENTATION},

     {[=, this](const wxCommandEvent& event)
      {
        if (
          auto* item = (show_message_item*)event.GetClientData();
          item != nullptr)
        {
          set_lsp_show_message(this, item);
          delete item;
        }
      },
      ID_LSP_SHOW_MESSAGE}});
}
