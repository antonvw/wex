////////////////////////////////////////////////////////////////////////////////
// Name:      statusbar.cpp
// Purpose:   Implementation of wex::statusbar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <map>
#include <optional>

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/factory/defs.h>
#include <wex/factory/frame.h>
#include <wex/ui/item-build.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/statusbar.h>

const int FIELD_NOT_SHOWN = -1;

namespace wex
{
/// This class contains all pane styles and some methods
/// to convert between lists and styles.
class pane_styles
{
public:
  /// Default constructor.
  pane_styles()
    : m_styles(
        {{wxSB_NORMAL, "normal"},
         {wxSB_FLAT, "flat"},
         {wxSB_RAISED, "raised"},
         {wxSB_SUNKEN, "sunken"}})
  {
    ;
  }

  /// Returns the list with this style at the front.
  config::strings_t find(int style) const
  {
    config::strings_t l;

    for (const auto& it : m_styles)
    {
      if (it.first == style)
      {
        l.push_front(it.second);
      }
      else
      {
        l.emplace_back(it.second);
      }
    }

    return l;
  }

  /// Returns the style for the first element on the list.
  int style(const config::strings_t& styles) const
  {
    if (const auto& it = std::ranges::find_if(
          m_styles,
          [styles](auto const& i)
          {
            return i.second == styles.front();
          });
        it != m_styles.end())
    {
      return it->first;
    }

    return wxSB_NORMAL;
  }

private:
  const std::map<int, std::string> m_styles;
};

std::vector<item> pane_dialog_items(const std::vector<statusbar_pane>& panes)
{
  std::vector<item> v_i(add_header({"pane", "width", "style"}));

  std::for_each(
    panes.begin(),
    panes.end(),
    [&v_i](const auto& it)
    {
      if (it.is_shown())
      {
        v_i.push_back(
          {find_after(it.name(), "Pane"), std::string(), item::STATICTEXT});

        v_i.push_back(
          {"statusbar.widths." + it.name(),
           item::TEXTCTRL_INT,
           std::to_string(it.GetWidth()),
           data::item(data::control()).label_type(data::item::LABEL_NONE)});

        v_i.push_back(
          {"statusbar.styles." + it.name(),
           item::COMBOBOX,
           pane_styles().find(it.GetStyle()),
           data::item(
             data::control().window(data::window().style(wxCB_READONLY)))
             .label_type(data::item::LABEL_NONE)});
      }
    });

  return v_i;
}

// The first pane is shown_pane_no, or FIELD_NOT_SHOWN if the pane is not
// shown, to be used as index in wxwidgets panes. The second pane the pane_no as
// index in the panes vector.
using pane_info_t = struct
{
  int shown_no;
  int no;
};

std::optional<pane_info_t>
pane_info(const std::vector<statusbar_pane>& panes, const std::string& pane)
{
  const std::string use_pane = pane.empty() ? "PaneText" : pane;

  for (int pane_no = 0, shown_pane_no = 0; const auto& it : panes)
  {
    if (it.is_shown())
    {
      if (it.name() == use_pane)
      {
        return pane_info_t{shown_pane_no, pane_no};
      }

      shown_pane_no++;
    }
    else
    {
      if (it.name() == use_pane)
      {
        return pane_info_t{FIELD_NOT_SHOWN, pane_no};
      }
    }

    pane_no++;
  }

  return {};
}
} // namespace wex

std::vector<wex::statusbar_pane> wex::statusbar::m_panes = {{}};

wex::statusbar::statusbar(factory::frame* parent, const data::window& data)
  : wxStatusBar(parent, data.id(), data.style(), data.name())
  , m_frame(parent)
{
  // The statusbar is not managed by Aui, so show/hide it explicitly.
  Show(config("show.StatusBar").get(true));

  Bind(
    wxEVT_CLOSE_WINDOW,
    [=, this](wxCloseEvent& event)
    {
      config("show.StatusBar").set(IsShown());
      event.Skip();
    });
}

const std::string wex::statusbar::get_statustext(const std::string& pane) const
{
  const auto& r(pane_info(m_panes, pane));
  return !r || r->shown_no == FIELD_NOT_SHOWN ?
           // Do not show error, as you might explicitly want to ignore
           // messages.
           std::string() :
           GetStatusText(r->shown_no).ToStdString();
}

void wex::statusbar::handle(wxMouseEvent& event, const statusbar_pane& pane)
{
  if (event.LeftUp())
  {
    if (pane.name() == "PaneText")
    {
      pane_dialog();
    }
    else
    {
      m_frame->statusbar_clicked(pane.name());
    }
  }
  else if (event.RightUp())
  {
    m_frame->statusbar_clicked_right(pane.name());
  }
  // Show tooltip if tooltip is available, and not yet presented.
  else if (event.Moving())
  {
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
  }
}

void wex::statusbar::on_mouse(wxMouseEvent& event)
{
  event.Skip();

  for (int pane_no = 0; const auto& it : m_panes)
  {
    if (it.is_shown())
    {
      if (wxRect rect; GetFieldRect(pane_no, rect))
      {
        if (rect.Contains(event.GetPosition()))
        {
          handle(event, it);
          return;
        }
      }

      pane_no++;
    }
  }
}

void wex::statusbar::pane_dialog()
{
  if (
    item_dialog(
      pane_dialog_items(m_panes),
      data::window().title("Statusbar Panes"),
      0,
      3) // we have 3 columns, see pane_dialog_items
      .ShowModal() == wxID_OK)
  {
    std::vector<statusbar_pane> v_p;
    std::transform(
      m_panes.begin(),
      m_panes.end(),
      std::back_inserter(v_p),
      [](const auto& it)
      {
        statusbar_pane p(
          it.name(),
          config("statusbar.widths." + it.name()).get(it.GetWidth()));
        p.help(it.help_text())
          .style(pane_styles().style(config("statusbar.styles." + it.name())
                                       .get(pane_styles().find(it.GetStyle()))))
          .show(it.is_shown());
        return p;
      });

    setup(m_frame, v_p);
  }
}

bool wex::statusbar::pane_show(const std::string& pane, bool show)
{
  assert(!m_panes.empty());

  auto* widths      = new int[m_panes.size()];
  auto* styles      = new int[m_panes.size()];
  bool  changed     = false;
  int   panes_shown = 0;

  std::vector<std::string> changes;

  for (auto& it : m_panes)
  {
    if (it.name() == pane)
    {
      if (show)
      {
        if (!it.is_shown())
        {
          changes.emplace_back(it.hidden_text());

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
        if (const int no = GetFieldsCount() - 1 - z; no >= 0)
        {
          SetStatusText(changes[j], no);
        }

        z++;
      }
    }
  }

  delete[] styles;
  delete[] widths;

  return !changes.empty() || changed;
}

bool wex::statusbar::set_statustext(
  const std::string& text,
  const std::string& pane)
{
  const auto& r(pane_info(m_panes, pane));

  if (!r)
  {
    // Do not show error, as you might explicitly want to ignore messages.
    return false;
  }

  if (r->shown_no == FIELD_NOT_SHOWN)
  {
    m_panes[r->no].hidden_text(text);
    return false;
  }

  m_panes[r->no].SetText(text);

  // wxStatusBar checks whether new text differs from current,
  // and does nothing if the same to avoid flicker.
  SetStatusText(text, r->shown_no);
  return true;
}

wex::statusbar* wex::statusbar::setup(
  factory::frame*                    frame,
  const std::vector<statusbar_pane>& panes,
  long                               style,
  const std::string&                 name)
{
  if (m_panes.size() > 1)
  {
    m_panes.clear();

    // If the PaneText is not present, add it as first pane.
    if (const auto& it = std::ranges::find_if(
          panes,
          [](const auto& p)
          {
            return p.name() == "PaneText";
          });
        it == panes.end())
    {
      m_panes.emplace_back();
    }
  }

  m_panes.insert(std::end(m_panes), std::begin(panes), std::end(panes));

  const bool first(frame->GetStatusBar() == nullptr);
  auto*      sb =
    (frame->GetStatusBar() == nullptr ?
       reinterpret_cast<statusbar*>(frame->CreateStatusBar(
         m_panes.size(),
         style,
         ID_UPDATE_STATUS_BAR,
         name)) :
       reinterpret_cast<statusbar*>(frame->GetStatusBar()));

  config::statusbar_t sb_def;

  for (const auto& it : m_panes)
  {
    sb_def.emplace_back(
      it.name(),
      pane_styles().find(it.GetStyle()),
      it.GetWidth());
  }

  const auto sb_config(config("statusbar").get(sb_def));
  config("statusbar").set(sb_config);

  int   panes_shown = 0;
  auto* styles      = new int[sb_config.size()];
  auto* widths      = new int[sb_config.size()];

  for (int i = 0; const auto& it : sb_config)
  {
    if (m_panes[i].is_shown())
    {
      styles[panes_shown] = pane_styles().style(std::get<1>(it));
      widths[panes_shown] = std::get<2>(it);

      m_panes[i].SetStyle(styles[panes_shown]);
      m_panes[i].SetWidth(widths[panes_shown]);
      panes_shown++;
    }

    i++;
  }

  sb->SetFieldsCount(panes_shown, widths);
  sb->SetStatusStyles(panes_shown, styles);

  delete[] styles;
  delete[] widths;

  if (first)
  {
    sb->Bind(wxEVT_LEFT_UP, &statusbar::on_mouse, sb);
    sb->Bind(wxEVT_RIGHT_UP, &statusbar::on_mouse, sb);
    sb->Bind(wxEVT_MOTION, &statusbar::on_mouse, sb);
  }

  return sb;
}
