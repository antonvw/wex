////////////////////////////////////////////////////////////////////////////////
// Name:      stc/dialog.cpp
// Purpose:   Implementation of class wex::stc_entry_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/stcdlg.h>
#include <wex/stc.h>

wex::stc_entry_dialog::stc_entry_dialog(
  const std::string& text,
  const std::string& prompt,
  const data::window& data)
  : dialog(data)
  , m_stc(new wex::stc(text, data::stc().window(data::window().parent(this))))
{
  if (!prompt.empty())
  {
    // See wxWidgets/src/generic/textdlgg.cpp, use similar bottom border flags.
    add_user_sizer(CreateTextSizer(prompt), 
      wxSizerFlags().DoubleBorder(wxBOTTOM));
  }

  m_stc->SetEdgeMode(wxSTC_EDGE_NONE);
  m_stc->SetName(data.title());
  m_stc->reset_margins();
  m_stc->SetViewEOL(false);
  m_stc->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  add_user_sizer(m_stc);

  layout_sizers();
}
