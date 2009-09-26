/******************************************************************************\
* File:          grid.h
* Purpose:       Declaration of wxExGrid class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXGRID_H
#define _EXGRID_H

#include <wx/grid.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/extension/menu.h> // for wxExMenu

#if wxUSE_GRID

class wxExFindReplaceDialog;

/// Offers popup menu with copy/paste, printing.
/// It also offers drag/drop functionality.
class wxExGrid : public wxGrid
{
public:
  /// Constructor.
  wxExGrid(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxWANTS_CHARS,
    const wxString& name = wxGridNameStr);

  // Interface.
#if wxUSE_DRAG_AND_DROP
  /// This one is invoked after IsAllowedDropSelection, and drops the data.
  /// Default it calls SetCellsValue.
  /// If you return true, the selection is correctly dropped,
  /// and the (old) selection is emptied and cleared to simulate dragging.
  virtual bool DropSelection(const wxGridCellCoords& drop_coords, const wxString& data);

  /// This one is invoked before dragging, and you can indicate
  /// whether you like the current selection to be dragged elsewhere.
  /// Default it is allowed.
  virtual bool IsAllowedDragSelection();

  /// This is invoked after dragging and before anything is really dropped.
  /// Default it checks for each cell whether it is read-only, etc.
  virtual bool IsAllowedDropSelection(const wxGridCellCoords& drop_coords, const wxString& data);
#endif

  /// This one is called by EmptySelection, SetCellsValue,
  /// and so during drag/drop as well, and allows you to
  /// override default here (which simply calls SetCellValue).
  /// So it is on a cell basis, whereas the DropSelection is on a range basis.
  virtual void SetGridCellValue(const wxGridCellCoords& coords, const wxString& data);

  /// Empties selected cells.
  void EmptySelection();

  /// Get text from selected cells,
  const wxString GetSelectedCellsValue();

  /// Copy from selected cells.
  bool CopySelectedCellsToClipboard();

  /// Paste starting at current grid cursor.
  void PasteCellsFromClipboard();

  /// Invokes wxExApp PrintText with BuildPage.
  void Print();

  /// Invokes wxExApp PreviewText with BuildPage.
  void PrintPreview();

  /// Fill cells with text starting at a cel.
  void SetCellsValue(const wxGridCellCoords& start_coords, const wxString& data);

#if wxUSE_DRAG_AND_DROP
  /// Specify whether you want to use drag/drop.
  /// Default it is used.
  void UseDragAndDrop(bool use);
#endif
protected:
  /// Builds the page used for printing.
  const wxString BuildPage();

  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  /// Finds next.
  bool FindNext(const wxString& text, bool find_next = true);

  /// Updates find replace text.
  void SetFindReplaceData();

  void OnCommand(wxCommandEvent& event);
  void OnFindDialog(wxFindDialogEvent& event);
  void OnGrid(wxGridEvent& event);
  void OnGridRange(wxGridRangeSelectEvent& event);
  void OnMouse(wxMouseEvent& event);
private:
#if wxUSE_DRAG_AND_DROP
  bool m_UseDragAndDrop;
#endif

  wxExFindReplaceDialog* m_FindReplaceDialog;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GRID

#endif
