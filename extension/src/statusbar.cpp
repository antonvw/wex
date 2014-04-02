////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wxExStatusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
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

const int FIELD_NOT_SHOWN = -1;

// #define DEBUG ON

#ifdef DEBUG
void DebugPanes(
  const wxString& field, 
  bool show,
  wxStatusBar* sb, 
  const std::vector<wxExStatusBarPane>& panes,
  const std::vector<wxString>& v)
{
  const int no = sb->GetFieldsCount() - 1 - v.size();
  
  wxLogMessage("field: %s show: %d start: %d", 
    field.c_str(),
    show,
    no);
    
  for (int l = 0; l < sb->GetFieldsCount(); l++)
  {
    wxLogMessage("statusbar[%d]=%s", l, sb->GetStatusText(l).c_str()); 
  }
  
  int i = 0;
  for (const auto& it : panes)
  {
    wxLogMessage("pane[%s](%d)=%s(shown: %d hidden: %s)", 
      it.GetName().c_str(),
      i++, 
      it.GetText().c_str(), 
      it.IsShown(), 
      it.GetHiddenText().c_str());
  }

  for (int k = 0; k < v.size(); k++)
  {
    wxLogMessage("changes[%d]=%s", k, v[k].c_str()); 
  }
}
#endif
  
void wxExStatusBarPane::Show(bool show)
{
  m_IsShown = show;
  
  if (show) 
  {
    m_HiddenText.clear();
  }
  else
  {
    m_HiddenText = GetText();
  }
}

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

bool wxExStatusBar::GetFieldNo(
  const wxString& field,
  int& shown_pane_no,
  int& pane_no) const
{
  shown_pane_no = 0;
  pane_no = 0;
  
  for (const auto& it : m_Panes)
  {
    if (it.IsShown())
    {
      if (it.GetName() == field)
      {
        return true;
      }
      
      shown_pane_no++;
    }
    else
    {
      if (it.GetName() == field)
      {
        shown_pane_no = FIELD_NOT_SHOWN;
        return true;
      }
    }
  
    pane_no++;
  }
  
  return false;
}
  
const wxString wxExStatusBar::GetStatusText(const wxString& field) const
{
  int shown_pane_no, dummy;
  
  if (!GetFieldNo(field, shown_pane_no, dummy) || shown_pane_no == FIELD_NOT_SHOWN)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return wxEmptyString;
  }
  
  return wxStatusBar::GetStatusText(shown_pane_no);
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

  int fieldno = 0;
  
  for (const auto& it : m_Panes)
  {
    if (it.IsShown())
    {
      wxRect rect;
      
      if (GetFieldRect(fieldno, rect))
      {
        if (rect.Contains(event.GetPosition()))
        {
          Handle(event, it);
          return;
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

  Bind(wxEVT_LEFT_UP, &wxExStatusBar::OnMouse, this, wxID_ANY);
  Bind(wxEVT_RIGHT_UP, &wxExStatusBar::OnMouse, this, wxID_ANY);
  Bind(wxEVT_MOTION, &wxExStatusBar::OnMouse, this, wxID_ANY);
}

bool wxExStatusBar::SetStatusText(const wxString& text, const wxString& field)
{
  int shown_pane_no, pane_no;

  if (!GetFieldNo(field, shown_pane_no, pane_no))
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }

  if (shown_pane_no == FIELD_NOT_SHOWN)
  {
    m_Panes[pane_no].SetHiddenText(text);
    return false;
  }
  else
  {
    m_Panes[pane_no].SetText(text);
    
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, shown_pane_no);
    return true;
  }
}

bool wxExStatusBar::ShowField(const wxString& field, bool show)
{
  wxASSERT(!m_Panes.empty());
  
  int* widths = new int[m_Panes.size()];
  int* styles = new int[m_Panes.size()];
  int panes_shown = 0;
  std::vector <wxString> changes;
  bool changed = false;

  for (auto& it : m_Panes)
  {
    if (it.GetName() == field)
    {
      if (show)
      {
        if (!it.IsShown())
        {
          changes.push_back(it.GetHiddenText());
          
          it.Show(true);
          
          for (int j = panes_shown; j < GetFieldsCount(); j++)
          {
            changes.push_back(wxStatusBar::GetStatusText(j));
          }
        }
        
        widths[panes_shown] = it.GetWidth();
        styles[panes_shown] = it.GetStyle();
        
        panes_shown++;
      }
      else
      {
        if (it.IsShown())
        {
          it.Show(false);
          changed = true;
          
          for (int j = panes_shown + 1; j < GetFieldsCount(); j++)
          {
            changes.push_back(wxStatusBar::GetStatusText(j));
          }
        }
      }
    }
    else
    {
      if (it.IsShown())
      {
        widths[panes_shown] = it.GetWidth();
        styles[panes_shown] = it.GetStyle();
        
        panes_shown++;
      }
    }
  }

  if (!changes.empty() || changed)
  {
    SetFieldsCount(panes_shown, widths);
    SetStatusStyles(panes_shown, styles);

    if (!changes.empty())
    {
      int z = 0;
      for (int j = changes.size() - 1; j >= 0; j--)
      {
        wxStatusBar::SetStatusText(changes[j], GetFieldsCount() - 1 - z);
        z++;
      }
    }
  }

#ifdef DEBUG
  DebugPanes(field, show, this, m_Panes, changes);
#endif

  delete[] styles;
  delete[] widths;
  
  return !changes.empty() || changed;
}

#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
