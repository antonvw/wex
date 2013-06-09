////////////////////////////////////////////////////////////////////////////////
// Name:      grid.cpp
// Purpose:   Implementation of wxExGrid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dnd.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/grid.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>

#if wxUSE_GRID
#if wxUSE_DRAG_AND_DROP
// A support class for implementing drag/drop on a grid.
class wxExTextDropTarget : public wxTextDropTarget
{
public:
  wxExTextDropTarget(wxExGrid* grid);
private:
  virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
  wxExGrid* m_Grid;
};

wxExTextDropTarget::wxExTextDropTarget(wxExGrid* grid)
  : wxTextDropTarget()
  , m_Grid(grid)
{
}

bool wxExTextDropTarget::OnDropText(
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
#endif // wxUSE_DRAG_AND_DROP

BEGIN_EVENT_TABLE(wxExGrid, wxGrid)
  EVT_FIND(wxID_ANY, wxExGrid::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExGrid::OnFindDialog)
  EVT_GRID_CELL_LEFT_CLICK(wxExGrid::OnGrid)
  EVT_GRID_CELL_RIGHT_CLICK(wxExGrid::OnGrid)
  EVT_GRID_CELL_BEGIN_DRAG(wxExGrid::OnGrid)
  EVT_GRID_SELECT_CELL(wxExGrid::OnGrid)
  EVT_GRID_RANGE_SELECT(wxExGrid::OnGridRange)
  EVT_MENU(wxID_DELETE, wxExGrid::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExGrid::OnCommand)
  EVT_MENU(ID_EDIT_SELECT_NONE, wxExGrid::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExGrid::OnCommand)
  EVT_MOUSE_EVENTS(wxExGrid::OnMouse)
END_EVENT_TABLE()

wxExGrid::wxExGrid(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxGrid(parent, id, pos, size, style, name)
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new wxExTextDropTarget(this));
  m_UseDragAndDrop = true;
#endif
}

const wxString wxExGrid::BuildPage()
{
  wxString text;

  text << "<TABLE ";

  if (GridLinesEnabled())
    text << "border=1";
  else
    text << "border=0";

  text << " cellpadding=4 cellspacing=0 >" << wxTextFile::GetEOL();

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
  // wxExClipboardAdd(text);

  return text;
}

void wxExGrid::BuildPopupMenu(wxExMenu& menu)
{
  menu.AppendEdit();
}

bool wxExGrid::CopySelectedCellsToClipboard() const
{
  wxBusyCursor wait;
  return wxExClipboardAdd(GetSelectedCellsValue());
}

#if wxUSE_DRAG_AND_DROP
bool wxExGrid::DropSelection(
  const wxGridCellCoords& drop_coords, 
  const wxString& data)
{
  SetCellsValue(drop_coords, data);

  return true;
}
#endif

void wxExGrid::EmptySelection()
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

bool wxExGrid::FindNext(const wxString& text, bool find_next)
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

  if (!wxExFindReplaceData::Get()->MatchCase())
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

      if (!wxExFindReplaceData::Get()->MatchCase())
      {
        text.MakeUpper();
      }

      if (wxExFindReplaceData::Get()->MatchWord())
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
    
    wxExFrame::StatusText(wxExGetFindResult(text, find_next, recursive), wxEmptyString);
    
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

const wxString wxExGrid::GetFindString() const
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
      wxExFindReplaceData::Get()->SetFindString(tkz.GetNextToken());
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
      wxExFindReplaceData::Get()->SetFindString(val);
    }
  }
    
  return wxExFindReplaceData::Get()->GetFindString();
}

const wxString wxExGrid::GetSelectedCellsValue() const
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
bool wxExGrid::IsAllowedDragSelection()
{
  return true;
}
#endif

#if wxUSE_DRAG_AND_DROP
bool wxExGrid::IsAllowedDropSelection(const wxGridCellCoords& drop_coords, const wxString& data)
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

void wxExGrid::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_COPY: CopySelectedCellsToClipboard(); break;
  case wxID_CUT: CopySelectedCellsToClipboard(); EmptySelection(); break;
  case wxID_DELETE: EmptySelection(); break;
  case wxID_PASTE: PasteCellsFromClipboard(); break;
  case wxID_SELECTALL: SelectAll(); break;
  case ID_EDIT_SELECT_NONE: ClearSelection(); break;
  default: wxFAIL;
  }
}

void wxExGrid::OnFindDialog(wxFindDialogEvent& event)
{
  if (
    event.GetEventType() == wxEVT_COMMAND_FIND ||
    event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
  {
    FindNext(
      wxExFindReplaceData::Get()->GetFindString(), 
      wxExFindReplaceData::Get()->SearchDown());
  }
  else
  {
    wxFAIL;
  }
}

void wxExGrid::OnGrid(wxGridEvent& event)
{
  if (event.GetEventType() == wxEVT_GRID_CELL_RIGHT_CLICK)
  {
    int style = (IsEditable() ? wxExMenu::MENU_DEFAULT: wxExMenu::MENU_IS_READ_ONLY);
    if (IsSelection()) style |= wxExMenu::MENU_IS_SELECTED;

    wxExMenu menu(style);
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
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      wxString::Format("%d,%d", 1 + event.GetCol(), 1 + event.GetRow()),
      "PaneInfo");
#endif

    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_GRID_CELL_BEGIN_DRAG)
  {
    // Readme: Not yet implemented.
  }
  else
  {
    wxFAIL;
  }
}

void wxExGrid::OnGridRange(wxGridRangeSelectEvent& event)
{
  event.Skip();

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(
    wxString::Format("%ld", GetSelectedCells().GetCount()),
    "PaneInfo");
#endif
}

void wxExGrid::OnMouse(wxMouseEvent& event)
{
  if (event.Dragging())
  {
    // Readme: This event is not coming in as well.
    // Could be used to trigger a real drag.
  }

  event.Skip();
}

void wxExGrid::PasteCellsFromClipboard()
{
  SetCellsValue(
    wxGridCellCoords(
      GetGridCursorRow(), 
      GetGridCursorCol()), 
    wxExClipboardGet());
}

void wxExGrid::Print()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  wxExPrinting::Get()->GetHtmlPrinter()->PrintText(BuildPage());
#endif
}

void wxExGrid::PrintPreview()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  wxExPrinting::Get()->GetHtmlPrinter()->PreviewText(BuildPage());
#endif
}

void wxExGrid::SetGridCellValue(
  const wxGridCellCoords& coords, 
  const wxString& data)
{
  SetCellValue(coords, data);
}

void wxExGrid::SetCellsValue(
  const wxGridCellCoords& start_coords, 
  const wxString& data)
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
void wxExGrid::UseDragAndDrop(bool use)
{
  m_UseDragAndDrop = use;
}
#endif

#endif //wxUSE_GRID
