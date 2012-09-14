////////////////////////////////////////////////////////////////////////////////
// Name:      process.cpp
// Purpose:   Implementation of class wxExProcessListView
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listitem.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listview.h>

wxExProcessListView::wxExProcessListView(wxExFrameWithHistory* frame)
  : wxExProcess()
  , m_Frame(frame)
  , m_ListView(NULL)
{
}

bool wxExProcessListView::ReportAdd(
  const wxString& line_with_eol) const
{
  if (m_ListView == NULL)
  {
    return false;
  }

  wxString line(line_with_eol);
  line.Trim();
  line.Trim(false);
  
  if (line.empty())
  {
    return false;
  }
  
  m_ListView->InsertItem(m_ListView->GetItemCount(), line);
  
  // If nothing selected, then ensure last line is visible.
  // Otherwise you are busy inspecting some line, and
  // it would be irritating to get it out of view.
  if (m_ListView->GetSelectedItemCount() == 0)
  {
    m_ListView->EnsureVisible(m_ListView->GetItemCount() - 1);
  }
  
#if wxUSE_STATUSBAR
  m_Frame->UpdateStatusBar(m_ListView);
#endif

  return true;
}

bool wxExProcessListView::ReportCreate()
{
  if (m_ListView == NULL)
  {
    m_ListView = m_Frame->Activate(wxExListViewFileName::LIST_PROCESS);
  }
  
  return m_ListView != NULL;
}

void wxExProcessListView::SetReport(wxExListView* report)
{
  m_ListView = report;
}
