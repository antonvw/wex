////////////////////////////////////////////////////////////////////////////////
// Name:      factory/frame.cpp
// Purpose:   Implementation of wex::factory::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/core.h>
#include <wex/factory/frame.h>
#include <wex/factory/grid.h>
#include <wex/factory/listview.h>
#include <wex/factory/stc.h>
#include <wex/lexers.h>
#include <wex/path.h>

#define wxCAST_TO(classname)                                     \
  if (m_find_focus != nullptr && m_find_focus->IsShown())        \
  {                                                              \
    if (classname* win = dynamic_cast<classname*>(m_find_focus); \
        win != nullptr)                                          \
    {                                                            \
      return win;                                                \
    }                                                            \
  }                                                              \
                                                                 \
  wxWindow*  win = wxWindow::FindFocus();                        \
  classname* cl  = dynamic_cast<classname*>(win);                \
  return cl;

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
    return stc->get_filename() == filename;
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
        wxPostEvent(lv, wxCommandEvent(wxEVT_MENU, wxID_JUMP_TO));
    }
  }
  else
  {
    // Clicking on another field, do nothing.
  }
}

bool wex::factory::frame::update_statusbar(const wxListView* lv)
{
  if (!m_is_closing && lv->IsShown())
  {
    const auto text = std::to_string(lv->GetItemCount()) +
                      (lv->GetSelectedItemCount() > 0 ?
                         "," + std::to_string(lv->GetSelectedItemCount()) :
                         std::string());

    return statustext(text, "PaneInfo");
  }

  return false;
}

// Do not make it const, too many const_casts needed,
bool wex::factory::frame::update_statusbar(stc* stc, const std::string& pane)
{
  if (stc == nullptr || m_is_closing)
  {
    return false;
  }

  std::stringstream text;

  if (pane == "PaneInfo")
  {
    if (!stc->is_visual())
    {
      text << stc->get_current_line() + 1 << "," << stc->get_line_count();
    }
    else if (stc->GetCurrentPos() == 0)
    {
      text << stc->get_line_count();
    }
    else
    {
      const auto line = stc->get_current_line() + 1;
      const auto pos =
        stc->GetCurrentPos() + 1 - stc->PositionFromLine(line - 1);
      int start, end;

      stc->GetSelection(&start, &end);

      if (const int len = end - start; len == 0)
      {
        text << line << "," << pos;
      }
      else
      {
        if (stc->SelectionIsRectangle())
        {
          text << line << "," << pos << "," << stc->GetSelectedText().length();
        }
        else
        {
          if (const auto number_of_lines =
                get_number_of_lines(stc->get_selected_text());
              number_of_lines <= 1)
            text << line << "," << pos << "," << len;
          else
            text << line << "," << number_of_lines << "," << len;
        }
      }
    }
  }
  else if (pane == "PaneLexer")
  {
    if (!lexers::get()->theme().empty())
    {
      text << stc->get_lexer().display_lexer();
    }
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
