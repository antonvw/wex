////////////////////////////////////////////////////////////////////////////////
// Name:      stc/dialog.cpp
// Purpose:   Implementation of class wex::stc_entry_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/factory/frame.h>
#include <wex/file-dialog.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/tostring.h>
#include <wex/util.h>

wex::stc_entry_dialog::stc_entry_dialog(
  const std::string&  text,
  const std::string&  prompt,
  const data::window& data)
  : dialog(data)
  , m_stc(new wex::stc(text, data::stc().window(data::window().parent(this))))
{
  if (!prompt.empty())
  {
    // See wxWidgets/src/generic/textdlgg.cpp, use similar bottom border flags.
    add_user_sizer(
      CreateTextSizer(prompt),
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

void wex::open_files_dialog(
  factory::frame*   frame,
  bool              ask_for_continue,
  const data::stc&  data,
  data::dir::type_t type)
{
  wxArrayString     paths;
  const std::string caption(_("Select Files"));
  bool              hexmode;

  if (auto* stc = dynamic_cast<wex::stc*>(frame->get_stc()); stc != nullptr)
  {
    file_dialog dlg(
      &stc->get_file(),
      data::window(data.window()).title(caption));

    if (ask_for_continue)
    {
      if (dlg.show_modal_if_changed(true) == wxID_CANCEL)
        return;
    }
    else
    {
      if (dlg.ShowModal() == wxID_CANCEL)
        return;
    }

    dlg.GetPaths(paths);
    hexmode = dlg.is_hexmode();
  }
  else
  {
    file_dialog dlg(data::window(data.window()).title(caption));

    if (dlg.ShowModal() == wxID_CANCEL)
      return;

    dlg.GetPaths(paths);
    hexmode = dlg.is_hexmode();
  }

  open_files(
    frame,
    to_vector_path(paths).get(),
    hexmode ? data::stc(data).flags(
                data::stc::window_t().set(data::stc::WIN_HEX),
                data::control::OR) :
              data,
    type);
}
