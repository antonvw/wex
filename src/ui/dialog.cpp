////////////////////////////////////////////////////////////////////////////////
// Name:      dialog.cpp
// Purpose:   Implementation of wex::dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/ui/dialog.h>

wex::dialog::dialog(const data::window& data)
  : wxDialog(
      data.parent(),
      data.id(),
      data.title().empty() ? "Dialog" : data.title(),
      data.pos(),
      data.size(),
      data.style() == data::NUMBER_NOT_SET ?
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER :
        data.style(),
      data.name().empty() ? "dialog" : data.name())
  , m_data(data)
  , m_top_sizer(new wxFlexGridSizer(1, 0, 0))
  , m_user_sizer(new wxFlexGridSizer(1, 0, 0))
{
}

wxSizerItem*
wex::dialog::add_user_sizer(wxWindow* window, const wxSizerFlags& flags)
{
  auto* item = m_user_sizer->Add(window, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_user_sizer->AddGrowableRow(m_user_sizer->GetChildren().GetCount() - 1);
  }

  return item;
}

wxSizerItem*
wex::dialog::add_user_sizer(wxSizer* sizer, const wxSizerFlags& flags)
{
  auto* item = m_user_sizer->Add(sizer, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_user_sizer->AddGrowableRow(m_user_sizer->GetChildren().GetCount() - 1);
  }

  return item;
}

void wex::dialog::layout_sizers(bool add_separator_line)
{
  m_top_sizer->AddGrowableCol(0);
  m_user_sizer->AddGrowableCol(0);

  const auto flag = wxSizerFlags().Expand().Border();

  // The top sizer starts with a spacer, for a nice border.
  m_top_sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  // Then place the growable user sizer.
  m_top_sizer->Add(m_user_sizer, flag);
  // So this is the user sizer, make the row growable.
  m_top_sizer->AddGrowableRow(m_top_sizer->GetChildren().GetCount() - 1);

  // Then, if buttons were specified, the button sizer.
  if (m_data.button() != 0)
  {
    if (wxSizer* sizer =
          (add_separator_line ? CreateSeparatedButtonSizer(m_data.button()) :
                                CreateButtonSizer(m_data.button()));
        sizer != nullptr)
    {
      m_top_sizer->Add(sizer, flag);
    }
  }

  // The top sizer ends with a spacer as well.
  m_top_sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  if (m_data.size() == wxDefaultSize)
  {
    SetSizerAndFit(m_top_sizer);
  }
  else
  {
    SetSizer(m_top_sizer);
  }
}
