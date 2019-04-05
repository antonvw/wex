////////////////////////////////////////////////////////////////////////////////
// Name:      dialog.cpp
// Purpose:   Implementation of wex::dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/dialog.h>

wex::dialog::dialog(const wex::window_data& data)
  : wxDialog(
      data.parent(),
      data.id(), 
      data.title().empty() ? "Dialog": data.title(), 
      data.pos(), 
      data.size(), 
      data.style() == DATA_NUMBER_NOT_SET ? 
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER: data.style(), 
      data.name().empty() ? "dialog": data.name())
  , m_Data(data)
  , m_TopSizer(new wxFlexGridSizer(1, 0, 0))
  , m_UserSizer(new wxFlexGridSizer(1, 0, 0))
{
}

wxSizerItem* wex::dialog::add_user_sizer(
  wxWindow* window,
  const wxSizerFlags& flags)
{
  wxSizerItem* item = m_UserSizer->Add(window, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_UserSizer->AddGrowableRow(m_UserSizer->GetChildren().GetCount() - 1);
  }

  return item;
}

wxSizerItem* wex::dialog::add_user_sizer(
  wxSizer* sizer,
  const wxSizerFlags& flags)
{
  wxSizerItem* item = m_UserSizer->Add(sizer, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_UserSizer->AddGrowableRow(m_UserSizer->GetChildren().GetCount() - 1);
  }

  return item;
}

void wex::dialog::layout_sizers(bool add_separator_line)
{
  m_TopSizer->AddGrowableCol(0);
  m_UserSizer->AddGrowableCol(0);

  const wxSizerFlags flag = wxSizerFlags().Expand().Border();

  // The top sizer starts with a spacer, for a nice border.
  m_TopSizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  // Then place the growable user sizer.
  m_TopSizer->Add(m_UserSizer, flag);
  // So this is the user sizer, make the row growable.
  m_TopSizer->AddGrowableRow(m_TopSizer->GetChildren().GetCount() - 1);

  // Then, if buttons were specified, the button sizer.
  if (m_Data.button() != 0)
  {
    if (wxSizer* sizer = (add_separator_line ?
      CreateSeparatedButtonSizer(m_Data.button()):
      CreateButtonSizer(m_Data.button()));
      sizer != nullptr)
    {
      m_TopSizer->Add(sizer, flag);
    }
  }

  // The top sizer ends with a spacer as well.
  m_TopSizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  if (m_Data.size() == wxDefaultSize)
  {
    SetSizerAndFit(m_TopSizer);
  }
  else
  {
    SetSizer(m_TopSizer);
  }
}
