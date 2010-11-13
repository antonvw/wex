/******************************************************************************\
* File:          renderer.cpp
* Purpose:       Implementation of wxExRenderer class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2007-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/renderer.h>

#if wxUSE_GUI
#if wxUSE_GRID

wxExRenderer::wxExRenderer(
  long flags,
  const wxPen& pen,
  const wxPen& pen_outer,
  const wxBrush& brush)
  : m_Brush(brush)
  , m_Pen(pen)
  , m_PenOuter(pen_outer)
  , m_Flags(flags)
{
}

void wxExRenderer::Draw(
  wxGrid& grid,
  wxGridCellAttr& attr,
  wxDC& dc,
  const wxRect& rect,
  int row,
  int col,
  bool isSelected)
{
  wxGridCellStringRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

  bool left = false;
  bool right = false;
  bool up = false;
  bool down = false;

  // These are drawn using m_PenOuter.
  if (m_PenOuter.GetWidth() > 0)
  {
    dc.SetPen(m_PenOuter);

    if (m_Flags & CELL_RECT_OUTER_UP)
    {
      up = true;
      dc.DrawLine(rect.GetTopLeft(), rect.GetTopRight());
    }

    if (m_Flags & CELL_RECT_OUTER_DOWN)
    {
      down = true;
      dc.DrawLine(rect.GetBottomLeft(), rect.GetBottomRight());
    }

    if (m_Flags & CELL_RECT_OUTER_LEFT)
    {
      left = true;
      dc.DrawLine(rect.GetTopLeft(), rect.GetBottomLeft());
    }

    if (m_Flags & CELL_RECT_OUTER_RIGHT)
    {
      right = true;
      dc.DrawLine(rect.GetTopRight(), rect.GetBottomRight());
    }

    if (m_Flags & CELL_RECT)
    {
      dc.SetBrush(m_Brush);
      dc.DrawRectangle(rect);
    }
  }

  // Rest is drawn using normal pen.
  if (m_Pen.GetWidth() == 0) return;

  dc.SetPen(m_Pen);

  if (up || down || left || right)
  {
    if (!up)    dc.DrawLine(rect.GetTopLeft(), rect.GetTopRight());
    if (!down)  dc.DrawLine(rect.GetBottomLeft(), rect.GetBottomRight());
    if (!left)  dc.DrawLine(rect.GetTopLeft(), rect.GetBottomLeft());
    if (!right) dc.DrawLine(rect.GetTopRight(), rect.GetBottomRight());
  }

  if (m_Flags & CELL_UP)
  {
    dc.DrawLine(rect.GetTopLeft(), rect.GetTopRight());
  }

  if (m_Flags & CELL_DOWN)
  {
    dc.DrawLine(rect.GetBottomLeft(), rect.GetBottomRight());
  }

  if (m_Flags & CELL_LEFT)
  {
    dc.DrawLine(rect.GetTopLeft(), rect.GetBottomLeft());
  }

  if (m_Flags & CELL_RIGHT)
  {
    dc.DrawLine(rect.GetTopRight(), rect.GetBottomRight());
  }

  if (m_Flags & CELL_CROSS)
  {
    dc.DrawLine(rect.GetTopLeft(), rect.GetBottomRight());
    dc.DrawLine(rect.GetBottomLeft(), rect.GetTopRight());
  }
}

#endif // wxUSE_GRID
#endif // wxUSE_GUI
