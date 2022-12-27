////////////////////////////////////////////////////////////////////////////////
// Name:      grid.cpp
// Purpose:   Implementation of wex::grid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/core.h>
#include <wex/data/find.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/factory/frame.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/ui/frd.h>
#include <wex/ui/grid.h>
#include <wx/app.h>
#include <wx/dnd.h>

#include <sstream>

namespace wex
{
// A support class for implementing drag/drop on a grid.
class text_droptarget : public wxTextDropTarget
{
public:
  explicit text_droptarget(grid* grid);

private:
  bool  OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
  grid* m_grid;
};

text_droptarget::text_droptarget(grid* grid)
  : wxTextDropTarget()
  , m_grid(grid)
{
}

bool text_droptarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
  const auto row = m_grid->YToRow(y - m_grid->GetColLabelSize());
  const auto col = m_grid->XToCol(x - m_grid->GetRowLabelSize());

  if (row == wxNOT_FOUND || col == wxNOT_FOUND)
  {
    return false;
  }

  const wxGridCellCoords coord(row, col);

  if (!m_grid->is_allowed_drop_selection(coord, data))
  {
    return false;
  }

  return m_grid->drop_selection(coord, data);
}
}; // namespace wex

wex::grid::grid(const data::window& data)
{
  Create(
    data.parent(),
    data.id(),
    data.pos(),
    data.size(),
    data.style(),
    data.name());

  SetDropTarget(new text_droptarget(this));

  lexers::get()->apply_default_style(
    [=, this](const std::string& back)
    {
      SetDefaultCellBackgroundColour(wxColour(back));
    },
    [=, this](const std::string& fore)
    {
      SetDefaultCellTextColour(wxColour(fore));
    });

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        empty_selection();
      },
      wxID_DELETE},
     {[=, this](wxCommandEvent& event)
      {
        SelectAll();
      },
      wxID_SELECTALL},
     {[=, this](wxCommandEvent& event)
      {
        ClearSelection();
      },
      ID_EDIT_SELECT_NONE},
     {[=, this](wxCommandEvent& event)
      {
        copy_selected_cells_to_clipboard();
      },
      wxID_COPY},
     {[=, this](wxCommandEvent& event)
      {
        copy_selected_cells_to_clipboard();
        empty_selection();
      },
      wxID_CUT},
     {[=, this](wxCommandEvent& event)
      {
        paste_cells_from_clipboard();
      },
      wxID_PASTE}});

  bind(this).frd(
    find_replace_data::get()->wx(),
    [=, this](const std::string& s, bool b)
    {
      data::find::recursive(false);
      data::find find(s, b);

      find_next(find);
    });

  Bind(
    wxEVT_GRID_CELL_LEFT_CLICK,
    [=, this](wxGridEvent& event)
    {
      // Removed extra check for !IsEditable(),
      // drag/drop is different from editing, so allow that.
      if (!IsSelection())
      {
        event.Skip();
        return;
      }

      if (m_use_drag_and_drop)
      {
        // This is because drag/drop is not really supported by the wxGrid.
        // Even the wxEVT_GRID_CELL_BEGIN_DRAG does not seem to come in.
        // Therefore, we are really dragging if you click again in
        // your selection and move mouse and drop elsewhere.
        // So, if not clicked in the selection, do nothing, this was no drag.
        if (!IsInSelection(event.GetRow(), event.GetCol()))
        {
          event.Skip();
          return;
        }

        // Start drag operation.
        wxTextDataObject textData(get_selected_cells_value());
        wxDropSource     source(textData, this);
        wxDragResult     result = source.DoDragDrop(wxDrag_DefaultMove);

        if (
          result != wxDragError && result != wxDragNone &&
          result != wxDragCancel)
        {
          // The old contents is not deleted, as should be by moving.
          // To fix this, do not call Skip so selection remains active,
          // and call empty_selection.
          //  event.Skip();
          empty_selection();
          ClearSelection();
        }
        else
        {
          // Do not call Skip so selection remains active.
          // event.Skip();
        }
      }
      else
      {
        event.Skip();
      }
    });

  Bind(
    wxEVT_GRID_CELL_RIGHT_CLICK,
    [=, this](wxGridEvent& event)
    {
      menu::menu_t style(menu::menu_t().set(menu::IS_POPUP));

      if (!IsEditable())
        style.set(wex::menu::IS_READ_ONLY);
      if (IsSelection())
        style.set(wex::menu::IS_SELECTED);

      wex::menu menu(style);
      build_popup_menu(menu);
      PopupMenu(&menu);
    });

  Bind(
    wxEVT_GRID_SELECT_CELL,
    [=, this](wxGridEvent& event)
    {
      auto* frame =
        dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
      frame->statustext(
        std::to_string(1 + event.GetCol()) + "," +
          std::to_string(1 + event.GetRow()),
        "PaneInfo");
      event.Skip();
    });

  Bind(
    wxEVT_GRID_RANGE_SELECT,
    [=, this](wxGridRangeSelectEvent& event)
    {
      event.Skip();
      auto* frame =
        dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
      frame->statustext(
        std::to_string(GetSelectedCells().GetCount()),
        "PaneInfo");
    });

  Bind(
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      auto* frame =
        dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
      if (frame != nullptr)
      {
        frame->set_find_focus(this);
      }
      event.Skip();
    });
}

const std::string wex::grid::build_page()
{
  std::stringstream text;

  text << "<TABLE ";

  if (GridLinesEnabled())
    text << "border=1";
  else
    text << "border=0";

  text << " cellpadding=4 cellspacing=0 >\n";
  text << "<tr>\n";

  // Add the col labels only if they are shown.
  if (GetColLabelSize() > 0)
  {
    for (int c = 0; c < GetNumberCols(); c++)
    {
      text << "<td><i>" << GetColLabelValue(c) << "</i>\n";
    }
  }

  for (int i = 0; i < GetNumberRows(); i++)
  {
    text << "<tr>\n";

    for (int j = 0; j < GetNumberCols(); j++)
    {
      text << "<td>"
           << (GetCellValue(i, j).empty() ? "&nbsp" : GetCellValue(i, j))
           << "\n";
    }
  }

  text << "</TABLE>\n";

  // This can be useful for testing, paste in a file and
  // check in your browser (there indeed rules are okay).
  // clipboard_add(text);

  return text.str();
}

void wex::grid::build_popup_menu(wex::menu& menu)
{
  menu.append({{menu_item::EDIT}});
}

bool wex::grid::copy_selected_cells_to_clipboard() const
{
  wxBusyCursor wait;
  return clipboard_add(get_selected_cells_value());
}

bool wex::grid::drop_selection(
  const wxGridCellCoords& drop_coords,
  const std::string&      data)
{
  set_cells_value(drop_coords, data);

  return true;
}

void wex::grid::empty_selection()
{
  wxBusyCursor wait;

  for (int i = 0; i < GetNumberRows(); i++)
  {
    for (int j = 0; j < GetNumberCols(); j++)
    {
      if (IsInSelection(i, j) && !IsReadOnly(i, j))
      {
        set_cell_value(wxGridCellCoords(i, j), std::string());
      }
    }
  }
}

bool wex::grid::find_next(const data::find& f)
{
  if (f.text().empty())
  {
    return false;
  }

  static int start_row;
  static int end_row;
  static int init_row;
  static int start_col;
  static int end_col;

  wxGridCellCoords grid_cursor(GetGridCursorRow(), GetGridCursorCol());

  if (f.is_forward())
  {
    init_row = 0;

    if (f.recursive())
    {
      start_row = init_row;
      start_col = 0;
    }
    else
    {
      start_row = grid_cursor.GetRow() + 1;
      start_col = grid_cursor.GetCol();
    }

    end_row = GetNumberRows();
    end_col = GetNumberCols();
  }
  else
  {
    init_row = GetNumberRows() - 1;

    if (f.recursive())
    {
      start_row = init_row;
      start_col = GetNumberCols() - 1;
    }
    else
    {
      start_row = grid_cursor.GetRow() - 1;
      start_col = grid_cursor.GetCol();
    }

    end_row = -1;
    end_col = -1;
  }

  if (start_col == -1)
  {
    start_col = 0;
  }

  if (start_row == -1)
  {
    start_row = 0;
  }

  wxGridCellCoords match;

  for (int j = start_col; j != end_col && !match; (f.is_forward() ? j++ : j--))
  {
    for (int i = (j == start_col ? start_row : init_row);
         i != end_row && !match;
         (f.is_forward() ? i++ : i--))
    {
      std::string cv = GetCellValue(i, j);

      if (!find_replace_data::get()->match_case())
      {
        boost::algorithm::to_upper(cv);
      }

      if (find_replace_data::get()->match_word())
      {
        if (cv == f.text())
        {
          match = wxGridCellCoords(i, j);
        }
      }
      else
      {
        if (cv.contains(f.text()))
        {
          match = wxGridCellCoords(i, j);
        }
      }
    }
  }

  if (!match)
  {
    bool result = false;

    f.statustext();

    if (!f.recursive())
    {
      f.recursive(true);
      result = find_next(f);
      f.recursive(false);
    }

    return result;
  }
  else
  {
    f.recursive(false);
    SetGridCursor(match.GetRow(), match.GetCol());
    MakeCellVisible(match); // though docs say this isn't necessary, it is
    return true;
  }
}

const std::string wex::grid::get_cells_value() const
{
  std::stringstream text;

  for (int i = 0; i < GetNumberRows(); i++)
  {
    bool value_added = false;

    for (int j = 0; j < GetNumberCols(); j++)
    {
      if (value_added)
      {
        text << "\t";
      }

      text << GetCellValue(i, j);

      value_added = true;
    }

    if (value_added)
    {
      text << "\n";
    }
  }

  return text.str();
}

const std::string wex::grid::get_find_string() const
{
  if (IsSelection())
  {
    // Only if we have one cell, so one EOL.
    if (boost::tokenizer<boost::char_separator<char>> tok(
          get_selected_cells_value(),
          boost::char_separator<char>("\n"));
        std::distance(tok.begin(), tok.end()) == 1)
    {
      find_replace_data::get()->set_find_string(*tok.begin());
    }
  }
  else
  {
    // Just take current cell value, if not empty.
    const auto        row = GetGridCursorRow();
    const auto        col = GetGridCursorCol();
    const std::string val = GetCellValue(row, col);

    if (!val.empty())
    {
      find_replace_data::get()->set_find_string(val);
    }
  }

  return find_replace_data::get()->get_find_string();
}

const std::string wex::grid::get_selected_cells_value() const
{
  // This does not work, only filled in for singly selected cells.
  // wxGridCellCoordsArray cells = GetSelectedCells();
  std::stringstream text;

  for (int i = 0; i < GetNumberRows(); i++)
  {
    bool value_added = false;

    for (int j = 0; j < GetNumberCols(); j++)
    {
      if (IsInSelection(i, j))
      {
        if (value_added)
        {
          text << "\t";
        }

        text << GetCellValue(i, j);

        value_added = true;
      }
    }

    if (value_added)
    {
      text << "\n";
    }
  }

  return text.str();
}

bool wex::grid::is_allowed_drop_selection(
  const wxGridCellCoords& drop_coords,
  const std::string&      data)
{
  auto start_at_row = drop_coords.GetRow();

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         data,
         boost::char_separator<char>("\n")))
  {
    int next_col = drop_coords.GetCol();

    boost::tokenizer<boost::char_separator<char>> tok(
      it,
      boost::char_separator<char>("\t"));

    for (auto tt = tok.begin(); tt != tok.end() && next_col < GetNumberCols();
         ++tt)
    {
      // If readonly, or this cell is part of the current selection, or outside
      // grid do not allow. Otherwise when dropping and clearing old selection
      // afterwards, we also cleared the new cells. If moving is really
      // supported by wxGrid, this might be changed.
      if (
        IsReadOnly(start_at_row, next_col) ||
        IsInSelection(start_at_row, next_col) ||
        start_at_row > GetNumberRows() || next_col > GetNumberCols())
      {
        return false;
      }

      next_col++;
    }

    start_at_row++;
  }

  return true;
}

void wex::grid::paste_cells_from_clipboard()
{
  set_cells_value(
    wxGridCellCoords(GetGridCursorRow(), GetGridCursorCol()),
    clipboard_get());
}

void wex::grid::print()
{
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PrintText(build_page());
}

void wex::grid::print_preview()
{
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PreviewText(build_page());
}

void wex::grid::set_cell_value(
  const wxGridCellCoords& coords,
  const std::string&      data)
{
  SetCellValue(coords, data);
}

void wex::grid::set_cells_value(
  const wxGridCellCoords& start_coords,
  const std::string&      data)
{
  for (auto        start_at_row = start_coords.GetRow();
       const auto& it : boost::tokenizer<boost::char_separator<char>>(
         data,
         boost::char_separator<char>("\n")))
  {
    boost::tokenizer<boost::char_separator<char>> tok(
      it,
      boost::char_separator<char>("\t"));

    auto next_col = start_coords.GetCol();

    for (auto t = tok.begin(); t != tok.end(); ++t)
    {
      if (!IsReadOnly(start_at_row, next_col))
      {
        set_cell_value(wxGridCellCoords(start_at_row, next_col), *t);
      }

      next_col++;
    }

    start_at_row++;
  }
}

void wex::grid::use_drag_and_drop(bool use)
{
  m_use_drag_and_drop = use;
}
