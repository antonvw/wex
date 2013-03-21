////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wxExStatusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/statusbar.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI
#if wxUSE_STATUSBAR

int wxExStatusBarPane::m_Total = 0;

BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_UP(wxExStatusBar::OnMouse)
  EVT_RIGHT_UP(wxExStatusBar::OnMouse)
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

int wxExStatusBar::FindField(const wxString& field) const
{
  for (
    const auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->GetName() == field)
    {
      return it->GetNo();
    }
  }
  
  return -1;
}
  
const wxString wxExStatusBar::GetStatusText(const wxString& field) const
{
  const int no = FindField(field);
  
  if (no == -1)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return wxEmptyString;
  }
  
  return wxStatusBar::GetStatusText(no);
}

void wxExStatusBar::Handle(wxMouseEvent& event, const wxExStatusBarPane& pane)
{
  if (event.LeftUp())
  {
    m_Frame->StatusBarClicked(pane.GetName());
  }
  else if (event.RightUp())
  {
    m_Frame->StatusBarClickedRight(pane.GetName());
  }
  // Show tooltip if tooltip is available, and not yet presented.
  else if (event.Moving())
  {
#if wxUSE_TOOLTIPS
    const wxString tooltip = GetToolTipText();
              
    if (pane.GetHelpText().empty())
    {
      if (!tooltip.empty())
      {
        UnsetToolTip();
      }
    }
    else if (tooltip != pane.GetHelpText())
    {
      SetToolTip(pane.GetHelpText());
    }
#endif    
  }
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
{
  event.Skip();

  bool found = false;

  for (int i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        found = true;

        for (
          auto it = m_Panes.begin();
          it != m_Panes.end();
          ++it)
        {
          // Handle the event, don't fail if none is true here,
          // it seems that moving and clicking almost at the same time
          // could cause assertions.
          if (it->IsShown() && it->GetNo() == i)
          {
            Handle(event, *it);
          }
        }
      }
    }
  }
}

void wxExStatusBar::SetFields(const std::vector<wxExStatusBarPane>& fields)
{
  wxASSERT(m_Panes.empty());
  
  m_Panes = fields;
    
  int* styles = new int[fields.size()];
  int* widths = new int[fields.size()];

  for (
    auto it = fields.begin();
    it != fields.end();
    ++it)
  {
    styles[it->GetNo()] = it->GetStyle();
    widths[it->GetNo()] = it->GetWidth();
  }
  
  SetFieldsCount(fields.size(), widths);
  SetStatusStyles(fields.size(), styles);

  delete[] styles;
  delete[] widths;

  Bind(
    wxEVT_MOTION,
    &wxExStatusBar::OnMouse,
    this,
    wxID_ANY);
}

bool wxExStatusBar::SetStatusText(const wxString& text, const wxString& field)
{
  const int no = FindField(field);

  if (no == -1)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }
  
  // wxStatusBar checks whether new text differs from current,
  // and does nothing if the same to avoid flicker.
  wxStatusBar::SetStatusText(text, no);
  
  return true;
}

bool wxExStatusBar::ShowField(const wxString& field, bool show)
{
  wxASSERT(!m_Panes.empty());
  
  const int pane = FindField(field);

  if (pane == -1)
  {
    return false;
  }
  
  bool changed = false;
  int* widths = new int[m_Panes.size()];
  int* styles = new int[m_Panes.size()];
  int i = 0;

  for (
    auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (pane == i)
    {
      if (show)
      {
        if (!it->IsShown())
        {
          widths[i] = it->GetWidth();
          styles[i] = it->GetStyle();
          it->Show(true);
          changed = true;
        }
        
        it->SetNo(i);
        i++;
      }
      else
      {
        if (it->IsShown())
        {
          it->Show(false);
          changed = true;
        }
      }
    }
    else
    {
      if (it->IsShown())
      {
        widths[i] = it->GetWidth();
        styles[i] = it->GetStyle();
        
        it->SetNo(i);
        i++;
      }
    }
  }

  if (changed)
  {
    SetFieldsCount(i, widths);
    SetStatusStyles(i, styles);
  }

  delete[] styles;
  delete[] widths;
  
  return changed;
}

#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
