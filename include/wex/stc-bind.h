////////////////////////////////////////////////////////////////////////////////
// Name:      stc-bind.h
// Purpose:   Declaration of bind id's
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/defs.h>
#include <wx/window.h>

namespace wex::id
{
  enum stc
  {
    beautify = wxID_HIGHEST + 1,
    edge_clear,
    edge_set,
    eol_dos,
    eol_unix,
    eol_mac,
    fold_all,
    hex,
    hex_dec_calltip,
    lowercase,
    margin_text_author,
    margin_text_date,
    margin_text_hide,
    margin_text_id,
    marker_next,
    marker_previous,
    open_link,
    open_mime,
    open_www,
    show_properties,
    toggle_fold,
    unfold_all,
    uppercase,
    vi_command,
    zoom_in,
    zoom_out,
  };
} // namespace wex::id
