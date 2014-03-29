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

const int FIELD_INVALID = -1;

//#define DEBUG ON

#ifdef DEBUG
void DebugPanes(const std::vector<wxExStatusBarPane>& panes)
{
  int i = 0;
  for (const auto& it : panes)
  {
    wxLogMessage("pane %d (%s): %s shown: %d hidden: %s", 
      i++, 
      it.GetName().c_str(),
      it.GetText().c_str(), 
      it.IsShown(), 
      it.GetHiddenText().c_str());
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

int wxExStatusBar::GetFieldNo(
  const wxString& field, bool& shown) const
{
  int i = 0;
  int j = 0;
  shown = false;
  
  for (const auto& it : m_Panes)
  {
    if (it.IsShown())
    {
      if (it.GetName() == field)
      {
        shown = true;
        return i;
      }
      
      i++;
    }
    else
    {
      if (it.GetName() == field)
      {
        return j;
      }
    }
  
    j++;
  }
  
  return FIELD_INVALID;
}
  
const wxString wxExStatusBar::GetStatusText(const wxString& field) const
{
  bool shown;
  const int no = GetFieldNo(field, shown);
  
  if (no == FIELD_INVALID || !shown)
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
  bool shown;
  const int no = GetFieldNo(field, shown);

  if (no == FIELD_INVALID)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }

  if (!shown)
  {
    m_Panes[no].SetHiddenText(text);
    return false;
  }
  else
  {
    m_Panes[no].SetText(text);
    
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, no);
    return true;
  }
}

bool wxExStatusBar::ShowField(const wxString& field, bool show)
{
  wxASSERT(!m_Panes.empty());
  
  int* widths = new int[m_Panes.size()];
  int* styles = new int[m_Panes.size()];
  int i = 0; // number of shown panes
  std::vector <wxString> changes;

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
          
          for (int j = i; j < GetFieldsCount(); j++)
          {
            changes.push_back(wxStatusBar::GetStatusText(j));
          }
        }
        
        widths[i] = it.GetWidth();
        styles[i] = it.GetStyle();
        
        i++;
      }
      else
      {
        if (it.IsShown())
        {
          it.Show(false);
          
          for (int j = i + 1; j < GetFieldsCount(); j++)
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
        widths[i] = it.GetWidth();
        styles[i] = it.GetStyle();
        
        i++;
      }
    }
  }

  if (!changes.empty())
  {
    SetFieldsCount(i, widths);
    SetStatusStyles(i, styles);
    
    int z = 0;
    for (int j = changes.size() - 1; j >= 0; j--)
    {
      wxStatusBar::SetStatusText(changes[j], GetFieldsCount() - 1 - z);
      z++;
    }
  
#ifdef DEBUG
    wxLogMessage("start: %d", GetFieldsCount() - 1 - z); 
      
    for (int k = 0; k < changes.size(); k++)
    {
      wxLogMessage("changes[%d]=%s", k, changes[k].c_str()); 
    }
  
    for (int l = 0; l < GetFieldsCount(); l++)
    {
      wxLogMessage("statusbar[%d]=%s", l, wxStatusBar::GetStatusText(l).c_str()); 
    }
  
    DebugPanes(m_Panes);
#endif
  }

  delete[] styles;
  delete[] widths;
  
  return !changes.empty();
}

#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
