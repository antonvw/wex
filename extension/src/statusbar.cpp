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

int wxExStatusBar::GetFieldNo(const wxString& field) const
{
  int i = 0;
  
  for (
    auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->IsShown())
    {
      if (it->GetName() == field)
      {
        return i;
      }
      
      i++;
    }
  }
  
  return -1;
}
  
const wxString wxExStatusBar::GetStatusText(const wxString& field) const
{
  const int no = GetFieldNo(field);
  
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
  int fieldno = 0;
  
  for (int i = 0; i < m_Panes.size() && !found; i++)
  {
    if (m_Panes[i].IsShown())
    {
      wxRect rect;
      
      if (GetFieldRect(fieldno, rect))
      {
        if (rect.Contains(event.GetPosition()))
        {
          found = true;

          Handle(event, m_Panes[i]);
        }
      }
      
      fieldno++;
    }
  }
}

void wxExStatusBar::SetFields(const std::vector<wxExStatusBarPane>& fields)
{
  m_Panes = fields;
    
  int* styles = new int[fields.size()];
  int* widths = new int[fields.size()];

  for (int i = 0; i < fields.size(); i++)
  {
    styles[i] = fields[i].GetStyle();
    widths[i] = fields[i].GetWidth();
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
  const int no = GetFieldNo(field);

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
  
  bool changed = false;
  int* widths = new int[m_Panes.size()];
  int* styles = new int[m_Panes.size()];
  int i = 0; // number of shown panes

  for (
    auto it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->GetName() == field)
    {
      if (show)
      {
        if (!it->IsShown())
        {
          it->Show(true);
          changed = true;
        }
        
        widths[i] = it->GetWidth();
        styles[i] = it->GetStyle();
        
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
