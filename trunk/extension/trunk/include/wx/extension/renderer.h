/******************************************************************************\
* File:          renderer.h
* Purpose:       Declaration of exRenderer class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2007-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_RENDERER_H
#define _EX_RENDERER_H

#include <wx/grid.h>
#include <wx/pen.h>
#include <wx/dc.h>

#if wxUSE_GUI

/// Offers a grid cell rendering.
class exRenderer : public wxGridCellStringRenderer
{
public:
  /// Renderer flags.
  enum exRendererFlags
  {
    CELL_UP               = 0x0001, ///< -
    CELL_DOWN             = 0x0002, ///< _
    CELL_LEFT             = 0x0004, ///< [
    CELL_RIGHT            = 0x0008, ///< ]
    CELL_CROSS            = 0x0010, ///< X
    CELL_RECT             = 0x0020, ///< []
    CELL_RECT_OUTER_UP    = 0x0100, ///< [-]
    CELL_RECT_OUTER_DOWN  = 0x0200, ///< [_]
    CELL_RECT_OUTER_LEFT  = 0x0400, ///< []
    CELL_RECT_OUTER_RIGHT = 0x0800, ///< []
  };

  /// Constructor.
  exRenderer(
    long flags,
    const wxPen& pen,                       ///< used for CELL_UP .. CELL_CROSS
    const wxPen& pen_outer,                 ///< used for CELL_RECT (outline) etc.
    const wxBrush& brush = *wxBLACK_BRUSH); ///< used for CELL_RECT filling

  /// Interface from wxGridCellRenderer.
  virtual void Draw(
    wxGrid& grid,
    wxGridCellAttr& attr,
    wxDC& dc,
    const wxRect& rect,
    int row,
    int col,
    bool isSelected);
private:
  wxBrush m_Brush;
  wxPen m_Pen;
  wxPen m_PenOuter;
  const long m_Flags;
};

#endif // wxUSE_GUI
#endif
