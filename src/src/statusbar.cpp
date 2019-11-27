////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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

void wex::statusbar_pane::show(bool show)
{
  m_is_shown = show;
  
  if (show) 
  {
    m_hidden.clear();
  }
  else
  {
    m_hidden = GetText();
  }
}

std::vector<wex::statusbar_pane> wex::statusbar::m_panes = {{}};

wex::statusbar::statusbar(frame* parent, const window_data& data)
  : wxStatusBar(parent, 
      data.id(), 
      data.style(), 
      data.name())
  , m_frame(parent)
{
  // The statusbar is not managed by Aui, so show/hide it explicitly.    
  Show(config("ShowStatusBar").get(true));
}

wex::statusbar::~statusbar()
{ 
  config("ShowStatusBar").set(IsShown());
}

// Returns a tuple with first field true if the specified field exists.
// The second field is shown_pane_no, or FIELD_NOT_SHOWN if the field is not shown,
// to be used as index in wxwidgets panes.
// The third field the pane_no as index in the panes vector.
std::tuple <bool, int, int> 
  wex::statusbar::field_info(const std::string& field) const
{
  const std::string use_field = field.empty() ? "PaneText": field;
  int pane_no = 0;
  int shown_pane_no = 0;
  
  for (const auto& it : m_panes)
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
  
const wex::statusbar_pane& wex::statusbar::get_field(int n) const
{
  return m_panes[n];
}

const std::string wex::statusbar::get_statustext(const std::string& field) const
{
  const auto& [res, shown_pane_no, pane_no] = field_info(field);
  return !res || shown_pane_no == FIELD_NOT_SHOWN ?
    // Do not show error, as you might explicitly want to ignore messages.
    std::string(): 
    GetStatusText(shown_pane_no).ToStdString();
}

void wex::statusbar::handle(wxMouseEvent& event, const statusbar_pane& pane)
{
  if (event.LeftUp())
  {
    m_frame->statusbar_clicked(pane.get_name());
  }
  else if (event.RightUp())
  {
    m_frame->statusbar_clicked_right(pane.get_name());
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

void wex::statusbar::on_mouse(wxMouseEvent& event)
{
  event.Skip();

  int fieldno = 0;
  
  for (const auto& it : m_panes)
  {
    if (it.is_shown())
    {
      if (wxRect rect; GetFieldRect(fieldno, rect))
      {
        if (rect.Contains(event.GetPosition()))
        {
          handle(event, it);
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
  const std::string& name)
{
  if (m_panes.size() > 1)
  {
    m_panes.clear();
  }

  m_panes.insert(std::end(m_panes), std::begin(panes), std::end(panes));

  statusbar* sb = (frame->GetStatusBar() == nullptr ?
    (statusbar *)frame->CreateStatusBar(
        m_panes.size(), style, ID_UPDATE_STATUS_BAR, name):
    (statusbar *)frame->GetStatusBar());

  config::statusbar_t sb_def;
  
  for (const auto& it : m_panes)
  {
    sb_def.push_back({it.get_name(), it.GetStyle(), it.GetWidth()});
  }
  
  const auto sb_config(config("Statusbar").get(sb_def));
  config("Statusbar").set(sb_config);

  auto* styles = new int[sb_config.size()];
  auto* widths = new int[sb_config.size()];

  int i = 0;
  for (const auto& it: sb_config)
  {
    styles[i] = std::get<1>(it);
    widths[i] = std::get<2>(it);

    m_panes[i].SetStyle(styles[i]);
    m_panes[i].SetWidth(widths[i]);
    i++;
  }
  
  sb->SetFieldsCount(m_panes.size(), widths);
  sb->SetStatusStyles(m_panes.size(), styles);

  delete[] styles;
  delete[] widths;

  sb->Bind(wxEVT_LEFT_UP, &statusbar::on_mouse, sb);
  sb->Bind(wxEVT_RIGHT_UP, &statusbar::on_mouse, sb);
  sb->Bind(wxEVT_MOTION, &statusbar::on_mouse, sb);

  return sb;
}

bool wex::statusbar::set_statustext(
  const std::string& text, const std::string& field)
{
  if (const auto& [res, shown_pane_no, pane_no] = field_info(field); !res)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }
  else if (shown_pane_no == FIELD_NOT_SHOWN)
  {
    m_panes[pane_no].set_hidden_text(text);
    return false;
  }
  else
  {
    m_panes[pane_no].SetText(text);
    
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    SetStatusText(text, shown_pane_no);
    return true;
  }
}

bool wex::statusbar::show_field(const std::string& field, bool show)
{
  assert(!m_panes.empty());
  
  auto* widths = new int[m_panes.size()];
  auto* styles = new int[m_panes.size()];
  int panes_shown = 0;
  std::vector<std::string> changes;
  bool changed = false;

  for (auto& it : m_panes)
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
