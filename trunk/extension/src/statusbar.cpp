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
#if wxUSE_TOOLTIPS
#include <wx/tooltip.h> // for GetTip
#endif
#include <wx/extension/statusbar.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI

int wxExPane::m_Total = 0;

#if wxUSE_STATUSBAR
BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(wxExStatusBar::OnMouse)
  EVT_LEFT_DCLICK(wxExStatusBar::OnMouse)
  EVT_MOTION(wxExStatusBar::OnMouse)
END_EVENT_TABLE()

wxExStatusBar::wxExStatusBar(
  wxExFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
}

const wxExPane wxExStatusBar::GetPane(int pane) const
{
  for (
    std::map<wxString, wxExPane>::const_iterator it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.m_No == pane)
    {
      return it->second;
    }
  }

  return wxExPane();
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
{
  bool found = false;

  for (int i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        found = true;

        // Handle the event, don't fail if none is true here,
        // it seems that moving and clicking almost at the same time
        // could cause assertions.
        if (event.ButtonDClick())
        {
          m_Frame->StatusBarDoubleClicked(GetPane(i).m_Name);
        }
        else if (event.ButtonDown())
        {
          m_Frame->StatusBarClicked(GetPane(i).m_Name);
        }
#if wxUSE_TOOLTIPS
        // Show tooltip if tooltip is available, and not yet tooltip presented.
        else if (event.Moving())
        {
          if (!m_Panes.empty())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != GetPane(i).m_Helptext)
            {
              SetToolTip(GetPane(i).m_Helptext);
            }
          }
        }
#endif
      }
    }
  }

  event.Skip();
}

void wxExStatusBar::SetPanes(const std::vector<wxExPane>& panes)
{
  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    std::vector<wxExPane>::const_iterator it = panes.begin();
    it != panes.end();
    ++it)
  {
    m_Panes[it->m_Name] = *it;
    styles[it->m_No] = it->GetStyle();
    widths[it->m_No] = it->GetWidth();
  }

  SetStatusStyles(panes.size(), styles);
  SetStatusWidths(panes.size(), widths);

  delete[] styles;
  delete[] widths;
}

void wxExStatusBar::SetStatusText(const wxString& text, const wxString& pane)
{
  std::map<wxString, wxExPane>::const_iterator it = m_Panes.find(pane);

  if (it != m_Panes.end())
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, it->second.m_No);
  }
}

#endif //wxUSE_STATUSBAR
#endif // wxUSE_GUI
