////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/statusbar.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/frame.h>

const int FIELD_NOT_SHOWN = -1;

std::string ConfigName(wex::statusbar* sb, const std::string& item, int f)
{
  return "SB" + sb->get_field(f).get_name() + item;
}

void wex::statusbar_pane::show(bool show)
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

std::vector<wex::statusbar_pane> wex::statusbar::m_Panes = {{}};

wex::statusbar::statusbar(frame* parent, const window_data& data)
  : wxStatusBar(parent, 
      data.id(), 
      data.style(), 
      data.name())
  , m_Frame(parent)
{
  // The statusbar is not managed by Aui, so show/hide it explicitly.    
  Show(config("ShowStatusBar").get(true));
}

wex::statusbar::~statusbar()
{ 
  config("ShowStatusBar").set(IsShown());

  for (int i = 0; i < GetFieldsCount(); i++)
  {
    config(ConfigName(this, "Style", i)).set(get_field(i).GetStyle());
    config(ConfigName(this, "Width", i)).set(get_field(i).GetWidth());
  }
}

const wex::statusbar_pane& wex::statusbar::get_field(int n) const
{
  return m_Panes[n];
}

// Returns true if the field exists.
// The shown_pane_no, pane_no is FIELD_NOT_SHOWN if the field is not shown.
std::tuple <bool, int, int> 
  wex::statusbar::GetFieldNo(const std::string& field) const
{
  const std::string use_field = field.empty() ? "PaneText": field;
  int pane_no = 0;
  int shown_pane_no = 0;
  
  for (const auto& it : m_Panes)
  {
    if (it.is_shown())
    {
      if (it.get_name() == use_field)
      {
        return {true, shown_pane_no, pane_no};
      }
      
      shown_pane_no++;
    }
    else
    {
      if (it.get_name() == use_field)
      {
        return {true, FIELD_NOT_SHOWN, pane_no};
      }
    }
  
    pane_no++;
  }
  
  return {false, 0, 0};
}
  
const std::string wex::statusbar::get_statustext(const std::string& field) const
{
  const auto& [res, shown_pane_no, pane_no] = GetFieldNo(field);
  return !res || shown_pane_no == FIELD_NOT_SHOWN ?
    // Do not show error, as you might explicitly want to ignore messages.
    std::string(): GetStatusText(shown_pane_no).ToStdString();
}

void wex::statusbar::Handle(wxMouseEvent& event, const statusbar_pane& pane)
{
  if (event.LeftUp())
  {
    m_Frame->statusbar_clicked(pane.get_name());
  }
  else if (event.RightUp())
  {
    m_Frame->statusbar_clicked_right(pane.get_name());
  }
  // Show tooltip if tooltip is available, and not yet presented.
  else if (event.Moving())
  {
#if wxUSE_TOOLTIPS
    if (const auto& tooltip = GetToolTipText(); pane.help_text().empty())
    {
      if (!tooltip.empty())
      {
        UnsetToolTip();
      }
    }
    else if (tooltip != pane.help_text())
    {
      SetToolTip(pane.help_text());
    }
#endif    
  }
}

void wex::statusbar::OnMouse(wxMouseEvent& event)
{
  event.Skip();

  int fieldno = 0;
  
  for (const auto& it : m_Panes)
  {
    if (it.is_shown())
    {
      if (wxRect rect; GetFieldRect(fieldno, rect))
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

wex::statusbar* wex::statusbar::setup(
  frame* frame,
  const std::vector<statusbar_pane>& panes,
  long style,
  const wxString& name)
{
  if (m_Panes.size() > 1)
  {
    m_Panes.clear();
  }

  m_Panes.insert(std::end(m_Panes), std::begin(panes), std::end(panes));

  statusbar* sb = (frame->GetStatusBar() == nullptr ?
    (statusbar *)frame->CreateStatusBar(
        m_Panes.size(), style, ID_UPDATE_STATUS_BAR, name):
    (statusbar *)frame->GetStatusBar());

  int* styles = new int[m_Panes.size()];
  int* widths = new int[m_Panes.size()];

  for (int i = 0; i < (int)m_Panes.size(); i++)
  {
    styles[i] = config(ConfigName(sb, "Style", i)).get(m_Panes[i].GetStyle());
    widths[i] = config(ConfigName(sb, "Width", i)).get(m_Panes[i].GetWidth());

    m_Panes[i].SetStyle(styles[i]);
    m_Panes[i].SetWidth(widths[i]);
  }
  
  sb->SetFieldsCount(m_Panes.size(), widths);
  sb->SetStatusStyles(m_Panes.size(), styles);

  delete[] styles;
  delete[] widths;

  sb->Bind(wxEVT_LEFT_UP, &statusbar::OnMouse, sb);
  sb->Bind(wxEVT_RIGHT_UP, &statusbar::OnMouse, sb);
  sb->Bind(wxEVT_MOTION, &statusbar::OnMouse, sb);

  return sb;
}

bool wex::statusbar::set_statustext(
  const std::string& text, const std::string& field)
{
  if (const auto& [res, shown_pane_no, pane_no] = GetFieldNo(field); !res)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }
  else if (shown_pane_no == FIELD_NOT_SHOWN)
  {
    m_Panes[pane_no].set_hidden_text(text);
    return false;
  }
  else
  {
    m_Panes[pane_no].SetText(text);
    
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    SetStatusText(text, shown_pane_no);
    return true;
  }
}

bool wex::statusbar::show_field(const std::string& field, bool show)
{
  assert(!m_Panes.empty());
  
  auto* widths = new int[m_Panes.size()];
  auto* styles = new int[m_Panes.size()];
  int panes_shown = 0;
  std::vector<std::string> changes;
  bool changed = false;

  for (auto& it : m_Panes)
  {
    if (it.get_name() == field)
    {
      if (show)
      {
        if (!it.is_shown())
        {
          changes.emplace_back(it.get_hidden_text());
          
          it.show(true);
          
          for (auto j = panes_shown; j < GetFieldsCount(); j++)
          {
            changes.emplace_back(GetStatusText(j));
          }
        }
        
        widths[panes_shown] = it.GetWidth();
        styles[panes_shown] = it.GetStyle();
        
        panes_shown++;
      }
      else
      {
        if (it.is_shown())
        {
          it.show(false);
          changed = true;
          
          for (auto j = panes_shown + 1; j < GetFieldsCount(); j++)
          {
            changes.emplace_back(GetStatusText(j));
          }
        }
      }
    }
    else
    {
      if (it.is_shown())
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
        SetStatusText(changes[j], GetFieldsCount() - 1 - z);
        z++;
      }
    }
  }

  delete[] styles;
  delete[] widths;
  
  return !changes.empty() || changed;
}
