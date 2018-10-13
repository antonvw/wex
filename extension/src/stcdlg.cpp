////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.cpp
// Purpose:   Implementation of class wex::stc_entry_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/persist/toplevel.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/stc.h>

wex::stc_entry_dialog::stc_entry_dialog(
  const std::string& text,
  const std::string& prompt,
  const window_data& data)
  : dialog(data)
  , m_STC(new stc(text, stc_data().Window(window_data().Parent(this))))
{
#if wxUSE_STATTEXT
  if (!prompt.empty())
  {
    // See wxWidgets: src\generic\textdlgg.cpp, use similar bottom border flags.
    AddUserSizer(CreateTextSizer(prompt), 
      wxSizerFlags().DoubleBorder(wxBOTTOM));
  }
#endif

  wxPersistentRegisterAndRestore(this);
  
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->SetName(data.Title());
  m_STC->ResetMargins();
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  AddUserSizer(m_STC);

  LayoutSizers();
}
