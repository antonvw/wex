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

int wxExPane::m_Total = 0;

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
  SendSizeEvent();
}

wxExStatusBar::~wxExStatusBar()
{ 
  wxConfigBase::Get()->Write("ShowStatusBar", IsShown());
}
  
const wxExPane wxExStatusBar::GetPane(int pane) const
{
  for (
    auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.GetNo() == pane)
    {
      return it->second;
    }
  }

  return wxExPane();
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

        const wxExPane& pane(GetPane(i));

        if (pane.GetNo() != -1)
        {
          // Handle the event, don't fail if none is true here,
          // it seems that moving and clicking almost at the same time
          // could cause assertions.
          if (event.ButtonDClick())
          {
            m_Frame->StatusBarDoubleClicked(pane.GetName());
          }
          else if (event.ButtonDown())
          {
            m_Frame->StatusBarClicked(pane.GetName());
          }
#if wxUSE_TOOLTIPS
          // Show tooltip if tooltip is available, and not yet tooltip presented.
          else if (event.Moving())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != pane.GetHelpText())
            {
              SetToolTip(pane.GetHelpText());
            }
          }
        }
#endif
      }
    }
  }
}

int wxExStatusBar::SetPanes(const std::vector<wxExPane>& panes)
{
  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    auto it = panes.begin();
    it != panes.end();
    ++it)
  {
    if (it->GetNo() != -1)
    {
      m_Panes.insert(std::make_pair(it->GetName(), *it));
      styles[it->GetNo()] = it->GetStyle();
      widths[it->GetNo()] = it->GetWidth();
    }
  }

  SetStatusStyles(m_Panes.size(), styles);
  SetStatusWidths(m_Panes.size(), widths);

  delete[] styles;
  delete[] widths;

  Bind(
    wxEVT_MOTION,
    &wxExStatusBar::OnMouse,
    this,
    wxID_ANY);

  return m_Panes.size();
}

bool wxExStatusBar::SetStatusText(const wxString& text, const wxString& pane)
{
  const auto it = m_Panes.find(pane);

  if (it != m_Panes.end())
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, it->second.GetNo());
    return true;
  }
  else
  {
    return false;
  }
}
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
