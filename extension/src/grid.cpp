////////////////////////////////////////////////////////////////////////////////
// Name:      grid.cpp
// Purpose:   Implementation of wex::grid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dnd.h>
#include <wx/extension/grid.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/printing.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>

namespace wex
{
  // A support class for implementing drag/drop on a grid.
  class text_droptarget : public wxTextDropTarget
  {
  public:
    explicit text_droptarget(grid* grid);
  private:
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
    grid* m_Grid;
  };

  text_droptarget::text_droptarget(grid* grid)
    : wxTextDropTarget()
    , m_Grid(grid)
  {
  }

  bool text_droptarget::OnDropText(
    wxCoord x, 
    wxCoord y, 
    const wxString& data)
  {
    const int row = m_Grid->YToRow(y - m_Grid->GetColLabelSize());
    const int col = m_Grid->XToCol(x - m_Grid->GetRowLabelSize());

    if (row == wxNOT_FOUND || col == wxNOT_FOUND)
    {
      return false;
    }

    const wxGridCellCoords coord(row, col);

    if (!m_Grid->IsAllowedDropSelection(coord, data))
    {
      return false;
    }

    return m_Grid->DropSelection(coord, data);
  }
};

wex::grid::grid(const window_data& data)
  : wxGrid(data.Parent(), data.Id(), data.Pos(), data.Size(), data.Style(), data.Name())
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new text_droptarget(this));
  m_UseDragAndDrop = true;
#endif

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    EmptySelection();}, wxID_DELETE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    SelectAll();}, wxID_SELECTALL);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    ClearSelection();}, ID_EDIT_SELECT_NONE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    CopySelectedCellsToClipboard();}, wxID_COPY);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    CopySelectedCellsToClipboard();
    EmptySelection();}, wxID_CUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    PasteCellsFromClipboard();}, wxID_PASTE);

  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    FindNext(
      find_replace_data::Get()->GetFindString(), 
      find_replace_data::Get()->SearchDown());});
      
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    FindNext(
      find_replace_data::Get()->GetFindString(), 
      find_replace_data::Get()->SearchDown());});

  Bind(wxEVT_GRID_CELL_LEFT_CLICK, [=](wxGridEvent& event) {
    // Removed extra check for !IsEditable(),
    // drag/drop is different from editing, so allow that.
    if (!IsSelection())
    {
      event.Skip();
      return;
    }

#if wxUSE_DRAG_AND_DROP
    if (m_UseDragAndDrop)
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

      // Is it allowed to drag current selection??
      if (!IsAllowedDragSelection())
      {
        event.Skip();
        return;
      }

      // Start drag operation.
      wxTextDataObject textData(GetSelectedCellsValue());
      wxDropSource source(textData, this);
      wxDragResult result = source.DoDragDrop(wxDrag_DefaultMove);

      if (result != wxDragError &&
          result != wxDragNone &&
          result != wxDragCancel)
      {
        // The old contents is not deleted, as should be by moving.
        // To fix this, do not call Skip so selection remains active,
        // and call EmptySelection.
        //  event.Skip();
        EmptySelection();
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
#else
    event.Skip();
#endif
    });
  
  Bind(wxEVT_GRID_CELL_RIGHT_CLICK, [=](wxGridEvent& event) {
    int style = (IsEditable() ? wex::menu::MENU_DEFAULT: wex::menu::MENU_IS_READ_ONLY);
    if (IsSelection()) style |= wex::menu::MENU_IS_SELECTED;

    wex::menu menu(style);
    BuildPopupMenu(menu);
    PopupMenu(&menu);
    });
    
  Bind(wxEVT_GRID_SELECT_CELL, [=](wxGridEvent& event) {
    frame::StatusText(
      std::to_string(1 + event.GetCol()) + "," + std::to_string(1 + event.GetRow()),
      "PaneInfo");
    event.Skip();});

  Bind(wxEVT_GRID_RANGE_SELECT, [=](wxGridRangeSelectEvent& event) {
    event.Skip();
    frame::StatusText(std::to_string(GetSelectedCells().GetCount()),
      "PaneInfo");
    });
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    wex::frame* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
    if (frame != nullptr)
    {
      frame->SetFindFocus(this);
    }
    event.Skip();});
}

const wxString wex::grid::BuildPage()
{
  wxString text;

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
    for (int c = 0 ; c < GetNumberCols(); c++)
    {
      text << "<td><i>" << GetColLabelValue(c) << "</i>\n";
    }
  }

  for (int i = 0 ; i < GetNumberRows(); i++)
  {
    text << "<tr>\n";

    for (int j = 0 ; j < GetNumberCols(); j++)
    {
      text << "<td>" <<
        (GetCellValue(i, j).empty() ? "&nbsp": GetCellValue(i, j)) << "\n";
    }
  }

  text << "</TABLE>\n";

  // This can be useful for testing, paste in a file and
  // check in your browser (there indeed rules are okay).
  // clipboard_add(text);

  return text;
}

void wex::grid::BuildPopupMenu(wex::menu& menu)
{
  menu.AppendEdit();
}

bool wex::grid::CopySelectedCellsToClipboard() const
{
  wxBusyCursor wait;
  return clipboard_add(GetSelectedCellsValue().ToStdString());
}

#if wxUSE_DRAG_AND_DROP
bool wex::grid::DropSelection(
  const wxGridCellCoords& drop_coords, 
  const wxString& data)
{
  SetCellsValue(drop_coords, data);

  return true;
}
#endif

void wex::grid::EmptySelection()
{
  wxBusyCursor wait;

  for (int i = 0; i < GetNumberRows(); i++)
  {
    for (int j = 0; j < GetNumberCols(); j++)
    {
      if (IsInSelection(i, j) && !IsReadOnly(i, j))
      {
        SetGridCellValue(wxGridCellCoords(i, j), wxEmptyString);
      }
    }
  }
}

bool wex::grid::FindNext(const wxString& text, bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static int start_row;
  static int end_row;
  static int init_row;
  static int start_col;
  static int end_col;

  wxString text_use = text;

  if (!find_replace_data::Get()->MatchCase())
  {
    text_use.MakeUpper();
  }

  wxGridCellCoords grid_cursor(GetGridCursorRow(), GetGridCursorCol());

  if (find_next)
  {
    init_row = 0;

    if (recursive)
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

    if (recursive)
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

  for (int j = start_col; j != end_col && !match; (find_next ? j++: j--))
  {
    for (int i = (j == start_col ? start_row: init_row);
         i != end_row && !match;
         (find_next ? i++: i--))
    {
      wxString text = GetCellValue(i, j);

      if (!find_replace_data::Get()->MatchCase())
      {
        text.MakeUpper();
      }

      if (find_replace_data::Get()->MatchWord())
      {
        if (text == text_use)
        {
          match = wxGridCellCoords(i, j);
        }
      }
      else
      {
        if (text.Contains(text_use))
        {
          match = wxGridCellCoords(i, j);
        }
      }
    }
  }

  if (!match)
  {
    bool result = false;
    
    frame::StatusText(
      get_find_result(text.ToStdString(), find_next, recursive), std::string());
    
    if (!recursive)
    {
      recursive = true;
      result = FindNext(text, find_next);
      recursive = false;
    }
    
    return result;
  }
  else
  {
    recursive = false;
    SetGridCursor(match.GetRow(), match.GetCol());
    MakeCellVisible(match); // though docs say this isn't necessary, it is
    return true;
  }
}

const wxString wex::grid::GetFindString() const
{
  if (IsSelection())
  {
    // This does not work (if single cell selected, array count is 0!
    // const wxGridCellCoordsArray cells(GetSelectedCells());
    tokenizer tkz(GetSelectedCellsValue().ToStdString(), "\n");

    // Only if we have one cell, so one EOL.
    if (tkz.CountTokens() == 1)
    {
      find_replace_data::Get()->SetFindString(tkz.GetNextToken());
    }
  }
  else
  {
    // Just take current cell value, if not empty.
    const int row = GetGridCursorRow();
    const int col = GetGridCursorCol();
    const wxString val = GetCellValue(row, col);

    if (!val.empty())
    {
      find_replace_data::Get()->SetFindString(val.ToStdString());
    }
  }
    
  return find_replace_data::Get()->GetFindString();
}

const wxString wex::grid::GetSelectedCellsValue() const
{
  // This does not work, only filled in for singly selected cells.
  // wxGridCellCoordsArray cells = GetSelectedCells();
  wxString text;

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

  return text;
}

#if wxUSE_DRAG_AND_DROP
bool wex::grid::IsAllowedDragSelection()
{
  return true;
}
#endif

#if wxUSE_DRAG_AND_DROP
bool wex::grid::IsAllowedDropSelection(const wxGridCellCoords& drop_coords, const wxString& data)
{
  tokenizer tkz(data.ToStdString(), "\n");

  int start_at_row = drop_coords.GetRow();

  while (tkz.HasMoreTokens())
  {
    const auto line(tkz.GetNextToken());

    tokenizer tkz(line, "\t");

    int next_col = drop_coords.GetCol();
    while (tkz.HasMoreTokens() && next_col < GetNumberCols())
    {
      tkz.GetNextToken(); // skip the value

      // If readonly, or this cell is part of the current selection, or outside grid
      // do not allow. Otherwise when dropping and clearing old selection afterwards,
      // we also cleared the new cells.
      // If moving is really supported by wxGrid, this might be changed.
      if (IsReadOnly(start_at_row, next_col) ||
          IsInSelection(start_at_row, next_col) ||
          start_at_row > GetNumberRows() ||
          next_col > GetNumberCols())
      {
        return false;
      }

      next_col++;
    }

    start_at_row++;
  }

  return true;
}
#endif

void wex::grid::PasteCellsFromClipboard()
{
  SetCellsValue(
    wxGridCellCoords(
      GetGridCursorRow(), 
      GetGridCursorCol()), 
    clipboard_get());
}

void wex::grid::Print()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  printing::Get()->GetHtmlPrinter()->PrintText(BuildPage());
#endif
}

void wex::grid::PrintPreview()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  printing::Get()->GetHtmlPrinter()->PreviewText(BuildPage());
#endif
}

void wex::grid::SetGridCellValue(
  const wxGridCellCoords& coords, 
  const wxString& data)
{
  SetCellValue(coords, data);
}

void wex::grid::SetCellsValue(
  const wxGridCellCoords& start_coords, 
  const wxString& data)
{
  tokenizer tkz(data.ToStdString(), "\n");

  int start_at_row = start_coords.GetRow();

  while (tkz.HasMoreTokens())
  {
    const auto line(tkz.GetNextToken());

    tokenizer tkz(line, "\t");

    int next_col = start_coords.GetCol();

    while (tkz.HasMoreTokens() && next_col < GetNumberCols())
    {
      const wxString value = tkz.GetNextToken();

      if (!IsReadOnly(start_at_row, next_col))
      {
        SetGridCellValue(wxGridCellCoords(start_at_row, next_col), value);
      }

      next_col++;
    }

    start_at_row++;
  }
}

#if wxUSE_DRAG_AND_DROP
void wex::grid::UseDragAndDrop(bool use)
{
  m_UseDragAndDrop = use;
}
#endif
