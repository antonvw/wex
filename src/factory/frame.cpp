////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wex::factory::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/path.h>
#include <wex/factory/frame.h>
#include <wex/factory/grid.h>
#include <wex/factory/listview.h>
#include <wex/factory/stc.h>

#define wxCAST_TO(classname)                                                   \
  /* NOLINTNEXTLINE */                                                         \
  if (m_find_focus != nullptr && m_find_focus->IsShown())                      \
  {                                                                            \
    if (classname* win = dynamic_cast<classname*>(m_find_focus);               \
        win != nullptr)                                                        \
    {                                                                          \
      return win;                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  wxWindow*  win = wxWindow::FindFocus();                                      \
  classname* cl  = dynamic_cast<classname*>(win);                              \
  return cl;

namespace wex
{
void update_paneinfo(factory::stc* stc, std::stringstream& text)
{
  if (stc->GetCurrentPos() == 0 && stc->get_line_count() != LINE_COUNT_UNKNOWN)
  {
    text << stc->get_line_count();
  }
  else
  {
    const auto line = stc->get_current_line() + 1;

    std::string show_pos;

    if (const auto pos = stc->GetCurrentPos() + 1 -
                         stc->PositionFromLine(stc->GetCurrentLine());
        pos > 0)
    {
      show_pos = "," + std::to_string(pos);
    }

    int start, end;

    stc->GetSelection(&start, &end);

    if (const int len = end - start; len == 0)
    {
      text << line << show_pos;
    }
    else
    {
      if (stc->SelectionIsRectangle())
      {
        text << line << show_pos << "," << stc->GetSelectedText().length();
      }
      else
      {
        if (const auto number_of_lines =
              get_number_of_lines(stc->get_selected_text());
            number_of_lines <= 1)
        {
          text << line << show_pos << "," << len;
        }
        else
        {
          text << line << "," << number_of_lines << "," << len;
        }
      }
    }
  }
}
} // namespace wex

wex::factory::frame::frame() = default;

wex::factory::frame::frame(
  wxWindow*          parent,
  wxWindowID         winid,
  const std::string& title)
  : wxFrame(parent, winid, title)
{
}

wex::factory::frame::~frame()
{
  m_is_closing = true;
}

wex::factory::grid* wex::factory::frame::get_grid()
{
  wxCAST_TO(wex::factory::grid);
}

wex::factory::listview* wex::factory::frame::get_listview()
{
  wxCAST_TO(wex::factory::listview);
}

wex::factory::stc* wex::factory::frame::get_stc()
{
  wxCAST_TO(wex::factory::stc);
}

bool wex::factory::frame::is_open(const wex::path& filename)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    return stc->path() == filename;
  }

  return false;
}

wex::factory::stc* wex::factory::frame::open_file(
  const wex::path&      filename,
  const wex::data::stc& data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->open(filename, data);
    return stc;
  }

  return nullptr;
}

wex::factory::stc* wex::factory::frame::open_file(
  const path&        filename,
  const std::string& text,
  const data::stc&   data)
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->set_text(text);
    return stc;
  }

  return nullptr;
}

void wex::factory::frame::SetMenuBar(wxMenuBar* bar)
{
  if (bar != nullptr)
  {
    m_menubar = bar;
  }

  wxFrame::SetMenuBar(
    !m_is_command && !config("show.MenuBar").get(true) ? nullptr : bar);
}

void wex::factory::frame::statusbar_clicked(const std::string& pane)
{
  if (auto* stc = get_stc(); pane == "PaneInfo")
  {
    if (stc != nullptr)
    {
      wxPostEvent(stc, wxCommandEvent(wxEVT_MENU, wxID_JUMP_TO));
    }
    else
    {
      if (auto* lv = get_listview(); lv != nullptr)
      {
        wxPostEvent(lv, wxCommandEvent(wxEVT_MENU, wxID_JUMP_TO));
      }
    }
  }
  else
  {
    // Clicking on another field, do nothing.
  }
}

bool wex::factory::frame::update_statusbar(const wxListView* lv)
{
  if (m_is_closing || lv == nullptr)
  {
    return false;
  }

  if (lv->IsShown())
  {
    const auto text = std::to_string(lv->GetItemCount()) +
                      (lv->GetSelectedItemCount() > 0 ?
                         "," + std::to_string(lv->GetSelectedItemCount()) :
                         std::string());

    return statustext(text, "PaneInfo");
  }

  return statustext(std::string(), std::string()) &&
         statustext(std::string(), "PaneInfo");
}

// Do not make it const, too many const_casts needed,
bool wex::factory::frame::update_statusbar(stc* stc, const std::string& pane)
{
  if (m_is_closing || stc == nullptr)
  {
    return false;
  }

  std::stringstream text;

  if (pane == "PaneInfo")
  {
    update_paneinfo(stc, text);
  }
  else if (pane.starts_with("PaneBlame"))
  {
    text << vcs_annotate_line(stc, pane);
  }
  else if (pane == "PaneLexer")
  {
    text << stc->lexer_name();
  }
  else if (pane == "PaneFileType")
  {
    switch (stc->GetEOLMode())
    {
      case wxSTC_EOL_CRLF:
        text << "DOS";
        break;
      case wxSTC_EOL_CR:
        text << "Mac";
        break;
      case wxSTC_EOL_LF:
        text << "Unix";
        break;
      default:
        assert(0);
    }
  }
  else if (pane == "PaneMode")
  {
    text << stc->vi_mode();
  }
  else
  {
    return false;
  }

  return statustext(text.str(), pane);
}
