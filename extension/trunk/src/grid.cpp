/******************************************************************************\
* File:          grid.cpp
* Purpose:       Implementation of exGrid class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: grid.cpp 16 2008-11-08 13:48:03Z anton $
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/dnd.h>
#include <wx/tokenzr.h>
#include <wx/extension/extension.h>
#include <wx/extension/grid.h>

#if wxUSE_GRID
#if wxUSE_DRAG_AND_DROP
// A support class for implementing drag/drop on a grid.
class exTextDropTarget : public wxTextDropTarget
{
public:
  exTextDropTarget(exGrid* grid);
private:
  virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
  exGrid* m_Grid;
};

exTextDropTarget::exTextDropTarget(exGrid* grid)
  : wxTextDropTarget()
  , m_Grid(grid)
{
}

bool exTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
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
#endif // wxUSE_DRAG_AND_DROP

BEGIN_EVENT_TABLE(exGrid, wxGrid)
  EVT_FIND(wxID_ANY, exGrid::OnFindDialog)
  EVT_FIND_CLOSE(wxID_ANY, exGrid::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, exGrid::OnFindDialog)
  EVT_GRID_CELL_LEFT_CLICK(exGrid::OnGrid)
  EVT_GRID_CELL_RIGHT_CLICK(exGrid::OnGrid)
  EVT_GRID_CELL_BEGIN_DRAG(exGrid::OnGrid)
  EVT_GRID_SELECT_CELL(exGrid::OnGrid)
  EVT_GRID_RANGE_SELECT(exGrid::OnGridRange)
  EVT_MENU_RANGE(wxID_CUT, wxID_PROPERTIES, exGrid::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_LOWEST, ID_EDIT_HIGHEST, exGrid::OnCommand)
  EVT_MOUSE_EVENTS(exGrid::OnMouse)
END_EVENT_TABLE()

exGrid::exGrid(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxGrid(parent, id, pos, size, style, name)
  , exInterface()
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new exTextDropTarget(this));
  m_UseDragAndDrop = true;
#endif

  wxAcceleratorEntry entries[3];
  entries[0].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[1].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
  entries[2].Set(wxACCEL_NORMAL, WXK_F5, wxID_FIND);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
}

const wxString exGrid::BuildPage()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  exApp::GetPrinter()->SetFooter(PrintFooter());
  exApp::GetPrinter()->SetHeader(PrintHeader());
#endif

  wxString text;

  text << "<TABLE border=1 cellpadding=4 cellspacing=0 ";

  // TODO: The rules does not work (also see exListView).
  if (GridLinesEnabled())
    text << "rules=\"all\" ";
  else
    text << "rules=\"none\" ";

  text << ">" << wxTextFile::GetEOL();

  text << "<tr>" << wxTextFile::GetEOL();

  // Add the col labels only if they are shown.
  if (GetColLabelSize() > 0)
  {
    for (int c = 0 ; c < GetNumberCols(); c++)
    {
      text << "<td><i>" << GetColLabelValue(c) << "</i>" << wxTextFile::GetEOL();
    }
  }

  for (int i = 0 ; i < GetNumberRows(); i++)
  {
    text << "<tr>" << wxTextFile::GetEOL();

    for (int j = 0 ; j < GetNumberCols(); j++)
    {
      text << "<td>" <<
        (GetCellValue(i, j).empty() ? "&nbsp": GetCellValue(i, j)) <<
        wxTextFile::GetEOL();
    }
  }

  text << "</TABLE>" << wxTextFile::GetEOL();

  // This can be useful for testing, paste in a file and
  // check in your browser (there indeed rules are okay).
  // exClipboardAdd(text);

  return text;
}

void exGrid::BuildPopupMenu(exMenu& menu)
{
  menu.Append(wxID_FIND);
  menu.AppendSeparator();
  menu.AppendEdit();
}

bool exGrid::CopySelectedCellsToClipboard()
{
  wxBusyCursor wait;
  return exClipboardAdd(GetSelectedCellsValue());
}

#if wxUSE_DRAG_AND_DROP
bool exGrid::DropSelection(const wxGridCellCoords& drop_coords, const wxString& data)
{
  SetCellsValue(drop_coords, data);

  return true;
}
#endif

void exGrid::EmptySelection()
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

void exGrid::FindDialog(wxWindow* parent, const wxString& caption)
{
  if (IsSelection())
  {
    // This does not work (if single cell selected, array count is 0!
    // const wxGridCellCoordsArray cells(GetSelectedCells());
    const wxString data = GetSelectedCellsValue();

    wxStringTokenizer tkz(data, wxTextFile::GetEOL());

    // Only if we have one cell, so one EOL.
    if (tkz.CountTokens() == 1)
    {
      exApp::GetConfig()->GetFindReplaceData()->SetFindString(tkz.GetNextToken());
    }
  }

  exInterface::FindDialog(parent, caption);
}

bool exGrid::FindNext(const wxString& text, bool find_next)
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

  if (!exApp::GetConfig()->GetFindReplaceData()->MatchCase())
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

  wxGridCellCoords match;

  for (int j = start_col; j != end_col && !match; (find_next ? j++: j--))
  {
    for (int i = (j == start_col ? start_row: init_row);
         i != end_row && !match;
         (find_next ? i++: i--))
    {
      wxString text = GetCellValue(i, j);

      if (!exApp::GetConfig()->GetFindReplaceData()->MatchCase())
      {
        text.MakeUpper();
      }

      if (exApp::GetConfig()->GetFindReplaceData()->MatchWord())
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
    return FindResult(text, find_next, recursive);
  }
  else
  {
    recursive = false;
    SetGridCursor(match.GetRow(), match.GetCol());
    MakeCellVisible(match); // though docs say this isn't necessary, it is
    return true;
  }
}

const wxString exGrid::GetSelectedCellsValue()
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
      text << wxTextFile::GetEOL();
    }
  }

  return text;
}

#if wxUSE_DRAG_AND_DROP
bool exGrid::IsAllowedDragSelection()
{
  return true;
}
#endif

#if wxUSE_DRAG_AND_DROP
bool exGrid::IsAllowedDropSelection(const wxGridCellCoords& drop_coords, const wxString& data)
{
  wxStringTokenizer tkz(data, wxTextFile::GetEOL());

  int start_at_row = drop_coords.GetRow();

  while (tkz.HasMoreTokens())
  {
    const wxString line = (tkz.GetNextToken());

    wxStringTokenizer tkz(line, "\t");

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

void exGrid::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_COPY: CopySelectedCellsToClipboard(); break;
  case wxID_CUT: CopySelectedCellsToClipboard(); EmptySelection(); break;
  case wxID_DELETE: EmptySelection(); break;
  case wxID_FIND: FindDialog(this); break;
  case wxID_PASTE: PasteCellsFromClipboard(); break;
  case wxID_SELECTALL: SelectAll(); break;
  case ID_EDIT_FIND_NEXT: FindNext(exApp::GetConfig()->GetFindReplaceData()->GetFindString()); break;
  case ID_EDIT_FIND_PREVIOUS: FindNext(exApp::GetConfig()->GetFindReplaceData()->GetFindString(), false); break;
  case ID_EDIT_SELECT_NONE: ClearSelection(); break;
  default: wxLogError(FILE_INFO("Unhandled"));
  }
}

void exGrid::OnFindDialog(wxFindDialogEvent& event)
{
  exInterface::OnFindDialog(event);
}

void exGrid::OnGrid(wxGridEvent& event)
{
  if (event.GetEventType() == wxEVT_GRID_CELL_RIGHT_CLICK)
  {
    int style = (IsEditable() ? exMenu::MENU_DEFAULT: exMenu::MENU_IS_READ_ONLY);
    if (IsSelection()) style |= exMenu::MENU_IS_SELECTED;

    exMenu menu(style);
    BuildPopupMenu(menu);
    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_GRID_CELL_LEFT_CLICK)
  {
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
  }
  else if (event.GetEventType() == wxEVT_GRID_SELECT_CELL)
  {
    exFrame::StatusText(
      wxString::Format("%d,%d", 1 + event.GetCol(), 1 + event.GetRow()),
      "PaneCells");

    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_GRID_CELL_BEGIN_DRAG)
  {
    wxLogMessage(FILE_INFO("Begin drag"));
  }
  else
  {
    wxLogError(FILE_INFO("Unhandled"));
  }
}

void exGrid::OnGridRange(wxGridRangeSelectEvent& event)
{
  event.Skip();

  exFrame::StatusText(
    wxString::Format("%d", GetSelectedCells().GetCount()),
    "PaneCells");
}

void exGrid::OnMouse(wxMouseEvent& event)
{
  // This event is not coming in as well.
  // Could be used to trigger a real drag.
  if (event.Dragging())
  {
    wxLogMessage(FILE_INFO("Is dragging"));
  }

  event.Skip();
}

void exGrid::PasteCellsFromClipboard()
{
  SetCellsValue(wxGridCellCoords(GetGridCursorRow(), GetGridCursorCol()), exClipboardGet());
}

void exGrid::SetGridCellValue(const wxGridCellCoords& coords, const wxString& data)
{
  SetCellValue(coords, data);
}

void exGrid::SetCellsValue(const wxGridCellCoords& start_coords, const wxString& data)
{
  wxStringTokenizer tkz(data, wxTextFile::GetEOL());

  int start_at_row = start_coords.GetRow();

  while (tkz.HasMoreTokens())
  {
    const wxString line = (tkz.GetNextToken());

    wxStringTokenizer tkz(line, "\t");

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
void exGrid::UseDragAndDrop(bool use)
{
  m_UseDragAndDrop = use;
}
#endif

#endif //wxUSE_GRID
