////////////////////////////////////////////////////////////////////////////////
// Name:      stc-bind.h
// Purpose:   Declaration of bind id's
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/window.h>

namespace wex::id
{
  class stc
  {
  public:
    static inline auto beautify           = wxWindow::NewControlId();
    static inline auto edge_clear         = wxWindow::NewControlId();
    static inline auto edge_set           = wxWindow::NewControlId();
    static inline auto eol_dos            = wxWindow::NewControlId(3);
    static inline auto eol_unix           = eol_dos + 1;
    static inline auto eol_mac            = eol_dos + 2;
    static inline auto fold_all           = wxWindow::NewControlId();
    static inline auto hex                = wxWindow::NewControlId();
    static inline auto hex_dec_calltip    = wxWindow::NewControlId();
    static inline auto lowercase          = wxWindow::NewControlId();
    static inline auto margin_text_author = wxWindow::NewControlId();
    static inline auto margin_text_date   = wxWindow::NewControlId();
    static inline auto margin_text_hide   = wxWindow::NewControlId();
    static inline auto margin_text_id     = wxWindow::NewControlId();
    static inline auto marker_next        = wxWindow::NewControlId(2);
    static inline auto marker_previous    = marker_next + 1;
    static inline auto open_link          = wxWindow::NewControlId();
    static inline auto open_mime          = wxWindow::NewControlId();
    static inline auto open_www           = wxWindow::NewControlId();
    static inline auto show_properties    = wxWindow::NewControlId();
    static inline auto toggle_fold        = wxWindow::NewControlId();
    static inline auto unfold_all         = wxWindow::NewControlId();
    static inline auto uppercase          = wxWindow::NewControlId();
    static inline auto vi_command         = wxWindow::NewControlId();
    static inline auto zoom_in            = wxWindow::NewControlId();
    static inline auto zoom_out           = wxWindow::NewControlId();
  };
} // namespace wex::id
