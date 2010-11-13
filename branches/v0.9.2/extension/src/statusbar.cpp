////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wxExStatusbar class
// Author:    Anton van Wezenbeek
// Created:   2010-03-26
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#if wxUSE_TOOLTIPS
#include <wx/tooltip.h> // for GetTip
#endif
#include <wx/extension/statusbar.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI
#if wxUSE_STATUSBAR

int wxExStatusBarPane::m_Total = 0;

BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(wxExStatusBar::OnMouse)
  EVT_LEFT_DCLICK(wxExStatusBar::OnMouse)
END_EVENT_TABLE()

wxExStatusBar::wxExStatusBar(
  wxExFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
  // The statusbar is not managed by Aui, so show/hide it explicitly.    
  Show(wxConfigBase::Get()->ReadBool("ShowStatusBar", true));
}

wxExStatusBar::~wxExStatusBar()
{ 
  wxConfigBase::Get()->Write("ShowStatusBar", IsShown());
}
  
const wxExStatusBarPane wxExStatusBar::GetField(int field) const
{
  for (
    auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.GetNo() == field)
    {
      return it->second;
    }
  }

  return wxExStatusBarPane();
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
{
  event.Skip();

  bool found = false;

  for (auto i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        found = true;

        const wxExStatusBarPane& field(GetField(i));

        if (field.GetNo() != -1)
        {
          // Handle the event, don't fail if none is true here,
          // it seems that moving and clicking almost at the same time
          // could cause assertions.
          if (event.ButtonDClick())
          {
            m_Frame->StatusBarDoubleClicked(field.GetName());
          }
          else if (event.ButtonDown())
          {
            m_Frame->StatusBarClicked(field.GetName());
          }
#if wxUSE_TOOLTIPS
          // Show tooltip if tooltip is available, and not yet presented.
          // Do not move check for NULL, otherwise no tooltip presented.
          else if (event.Moving())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != field.GetHelpText())
            {
              SetToolTip(field.GetHelpText());
            }
          }
#endif
        }
      }
    }
  }
}

int wxExStatusBar::SetFields(const std::vector<wxExStatusBarPane>& fields)
{
  wxASSERT(m_Panes.empty());
  
  int* styles = new int[fields.size()];
  int* widths = new int[fields.size()];

  for (
    auto it = fields.begin();
    it != fields.end();
    ++it)
  {
    // our member should have same size as fields
    m_Panes.insert(std::make_pair(it->GetName(), *it));
    
    if (it->GetNo() != -1)
    {
      styles[it->GetNo()] = it->GetStyle();
      widths[it->GetNo()] = it->GetWidth();
    }
  }
  
  if (GetFieldsCount() == 0)
  {
    SetFieldsCount(m_Panes.size(), widths);
  }
  else
  {
    SetStatusWidths(m_Panes.size(), widths);
  }

  SetStatusStyles(m_Panes.size(), styles);

  delete[] styles;
  delete[] widths;

  Bind(
    wxEVT_MOTION,
    &wxExStatusBar::OnMouse,
    this,
    wxID_ANY);

  return m_Panes.size();
}

bool wxExStatusBar::SetStatusText(const wxString& text, const wxString& field)
{
  const auto it = m_Panes.find(field);

  if (it != m_Panes.end())
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, it->second.GetNo());
    return true;
  }
  else
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }
}
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
