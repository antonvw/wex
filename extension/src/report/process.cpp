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
  const wxString& line, 
  const wxString& path,
  const wxString& lineno)
{
  if (m_ListView == NULL)
  {
    return false;
  }

  if (!path.empty())
  {  
    wxFileName fn(path);
    fn.Normalize();
    
    if (fn.FileExists() || fn.DirExists())
    {
      wxExListItem item(m_ListView, fn);
      item.Insert();
      item.SetItem(_("Line"), line);
      item.SetItem(_("Line No"), lineno);
    }
    else
    {
      m_ListView->InsertItem(m_ListView->GetItemCount(), line);
    }
  }
  else
  {
    m_ListView->InsertItem(m_ListView->GetItemCount(), line);
  }
  
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
  m_ListView = m_Frame->Activate(wxExListViewFileName::LIST_PROCESS);
  
  return m_ListView != NULL;
}
