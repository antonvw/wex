////////////////////////////////////////////////////////////////////////////////
// Name:      dialog.cpp
// Purpose:   Implementation of wxExDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dialog.h>

#if wxUSE_GUI
wxExDialog::wxExDialog(wxWindow* parent,
  const wxString& title,
  long button_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size, 
  long style,
  const wxString& name)
  : wxDialog(parent, id, title, pos, size, style, name)
  , m_ButtonFlags(button_flags)
  , m_HasDefaultSize(size == wxDefaultSize)
  , m_TopSizer(new wxFlexGridSizer(1, 0, 0))
  , m_UserSizer(new wxFlexGridSizer(1, 0, 0))
{
}

wxSizerItem* wxExDialog::AddUserSizer(
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

wxSizerItem* wxExDialog::AddUserSizer(
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

void wxExDialog::LayoutSizers(bool add_separator_line)
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
  if (m_ButtonFlags != 0)
  {
    wxSizer* sizer = (add_separator_line ?
      CreateSeparatedButtonSizer(m_ButtonFlags):
      CreateButtonSizer(m_ButtonFlags));

    if (sizer != NULL)
    {
      m_TopSizer->Add(sizer, flag);
    }
  }

  // The top sizer ends with a spacer as well.
  m_TopSizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  if (m_HasDefaultSize)
  {
    SetSizerAndFit(m_TopSizer);
  }
  else
  {
    SetSizer(m_TopSizer);
  }
}

#endif // wxUSE_GUI
