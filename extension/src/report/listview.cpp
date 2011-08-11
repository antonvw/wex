////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of class 'wxExListViewWithFrame'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listitem.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcs.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

BEGIN_EVENT_TABLE(wxExListViewWithFrame, wxExListViewFileName)
  EVT_MENU_RANGE(ID_LIST_LOWEST, ID_LIST_HIGHEST, wxExListViewWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, wxExListViewWithFrame::OnCommand)
  EVT_MENU_RANGE(
    ID_EDIT_VCS_LOWEST, 
    ID_EDIT_VCS_HIGHEST, 
    wxExListViewWithFrame::OnCommand)
END_EVENT_TABLE()

wxExListViewWithFrame::wxExListViewWithFrame(wxWindow* parent,
  wxExFrameWithHistory* frame,
  ListType type,
  wxWindowID id,
  long menu_flags,
  const wxExLexer* lexer,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString &name)
  : wxExListViewFileName(
      parent, 
      type, 
      id, 
      lexer, 
      pos, 
      size, 
      style, 
      validator, 
      name)
  , m_Frame(frame)
  , m_MenuFlags(menu_flags)
{
  if (GetType() == LIST_HISTORY)
  {
    m_Frame->UseFileHistoryList(this);
  }
}

void wxExListViewWithFrame::BuildPopupMenu(wxExMenu& menu)
{
  bool exists = true;
  bool is_folder = false;
  bool read_only = false;
  bool is_make = false;

  if (GetSelectedItemCount() >= 1)
  {
    const wxExListItem item(this, GetFirstSelected());

    exists = item.GetFileName().GetStat().IsOk();
    is_folder = wxFileName::DirExists(item.GetFileName().GetFullPath());
    read_only = item.GetFileName().GetStat().IsReadOnly();
    is_make = item.GetFileName().GetLexer().GetScintillaLexer() == "makefile";
  }

  if (GetSelectedItemCount() >= 1 && exists)
  {
    menu.Append(ID_LIST_OPEN_ITEM, _("&Open"), wxART_FILE_OPEN);
    menu.AppendSeparator();
  }

  wxExListViewFileName::BuildPopupMenu(menu);

  if (GetSelectedItemCount() > 1 && exists &&
     !wxConfigBase::Get()->Read(_("Comparator")).empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_LIST_COMPARE, _("C&ompare"));
  }

  if (GetSelectedItemCount() == 1)
  {
    if (is_make)
    {
      menu.AppendSeparator();
      menu.Append(ID_LIST_RUN_MAKE, _("&Make"));
    }

    if ( GetType() != LIST_FILE &&
        !wxExVCS().Use() &&
         exists && !is_folder)
    {
      wxExListView* list = m_Frame->Activate(LIST_FILE);

      if (list != NULL && list->GetSelectedItemCount() == 1)
      {
        wxExListItem thislist(this, GetFirstSelected());
        const wxString current_file = thislist.GetFileName().GetFullPath();

        wxExListItem otherlist(list, list->GetFirstSelected());
        const wxString with_file = otherlist.GetFileName().GetFullPath();

        if (current_file != with_file &&
            !wxConfigBase::Get()->Read(_("Comparator")).empty())
        {
          menu.AppendSeparator();
          menu.Append(ID_LIST_COMPARE,
            _("&Compare With") + " " + wxExGetEndOfText(with_file));
        }
      }
    }
  }

  if (GetSelectedItemCount() >= 1)
  {
    if (exists && !is_folder)
    {
      if (!wxExVCS().Use())
      {
        menu.AppendSeparator();

        if (!wxConfigBase::Get()->Read(_("Comparator")).empty())
        {
          menu.Append(ID_LIST_COMPARELAST, _("&Compare Recent Version"));
        }

        menu.Append(ID_LIST_VERSIONLIST, _("&Version List"));
      }
      
      if (wxExVCS::DirExists(
        wxExListItem(this, GetFirstSelected()).GetFileName()))
      {
        menu.AppendSeparator();
        menu.AppendVCS(wxExListItem(this, GetFirstSelected()).GetFileName());
      }
    }

    // Finding in the LIST_FIND and REPLACE would 
    /// result in recursive calls, do not add them.
    if ( exists &&
         GetType() != LIST_FIND && GetType() != LIST_REPLACE &&
        (m_MenuFlags & LIST_MENU_REPORT_FIND))
    {
      menu.AppendSeparator();
      menu.Append(ID_TOOL_REPORT_FIND, 
        wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

      if (!read_only)
      {
        menu.Append(ID_TOOL_REPORT_REPLACE, 
          wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_REPLACE)));
      }
    }
  }

  if (GetSelectedItemCount() > 0 && 
      exists && 
     (m_MenuFlags & LIST_MENU_TOOL) &&
      wxExLexers::Get()->Count() > 0)
  {
    menu.AppendSeparator();
    menu.AppendTools();
  }
}

wxExListViewWithFrame::ListType wxExListViewWithFrame::GetTypeTool(
  const wxExTool& tool)
{
  switch (tool.GetId())
  {
    case ID_TOOL_REPORT_COUNT: return LIST_COUNT; break;
    case ID_TOOL_REPORT_FIND: return LIST_FIND; break;
    case ID_TOOL_REPORT_KEYWORD: return LIST_KEYWORD; break;
    case ID_TOOL_REPORT_REPLACE: return LIST_REPLACE; break;
    case ID_TOOL_REPORT_REVISION: return LIST_REVISION; break;
    case ID_TOOL_REPORT_SQL: return LIST_SQL; break;
    case ID_TOOL_REPORT_VERSION: return LIST_VERSION; break;
    default: wxFAIL; return LIST_FILE;
  }
}

void wxExListViewWithFrame::ItemActivated(long item_number)
{
  const wxExListItem item(this, item_number);

  if (item.GetFileName().FileExists())
  {
    const wxString line_number_str = GetItemText(item_number, _("Line No"));
    const auto line_number = atoi(line_number_str.c_str());
    const wxString match =
      (GetType() == LIST_REPLACE ?
         GetItemText(item_number, _("Replaced")):
         GetItemText(item_number, _("Match")));

    m_Frame->OpenFile(
      item.GetFileName().GetFullPath(),
      line_number, match);

    SetFocus();
  }
  else
  { 
    wxExListViewFileName::ItemActivated(item_number);
  }
}

void wxExListViewWithFrame::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() > ID_TOOL_LOWEST && event.GetId() < ID_TOOL_HIGHEST)
  {
    RunItems(event.GetId());
  }
  else if (event.GetId() > ID_EDIT_VCS_LOWEST && event.GetId() < ID_EDIT_VCS_HIGHEST)
  {
    wxArrayString files;
    
    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      files.Add(wxExListItem(this, i).GetFileName().GetFullPath());
    }
  
    wxExVCSExecute(m_Frame, event.GetId(), files);
  }
  else switch (event.GetId())
  {
  case ID_LIST_OPEN_ITEM:
  {
    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
      ItemActivated(i);
  }
  break;

  case ID_LIST_COMPARE:
  case ID_LIST_COMPARELAST:
  case ID_LIST_VERSIONLIST:
  {
    bool first = true;
    wxString file1,file2;
    bool found = false;

    wxExListViewFileName* list = NULL;

    if (event.GetId() == ID_LIST_VERSIONLIST)
    {
      list = m_Frame->Activate(LIST_VERSION);
      wxASSERT(list != NULL);
    }

    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      wxExListItem li(this, i);
      const wxFileName* filename = &li.GetFileName();
      if (wxFileName::DirExists(filename->GetFullPath())) continue; // IsDir no ok
      switch (event.GetId())
      {
        case ID_LIST_COMPARE:
        {
          if (GetSelectedItemCount() == 1)
          {
            list = m_Frame->Activate(LIST_FILE);
            if (list == NULL) return;
            const auto main_selected = list->GetFirstSelected();
            wxExCompareFile(wxExListItem(list, main_selected).GetFileName(), *filename);
          }
          else
          {
            if (first)
            {
              first = false;
              file1 = filename->GetFullPath();
            }
            else
            {
              first = true;
              file2 = filename->GetFullPath();
            }
            if (first) wxExCompareFile(wxFileName(file1), wxFileName(file2));
          }
        }
        break;
        case ID_LIST_COMPARELAST:
        {
          wxFileName lastfile;
          if (wxExFindOtherFileName(*filename, &lastfile))
          {
            wxExCompareFile(*filename, lastfile);
          }
        }
        break;
        case ID_LIST_VERSIONLIST:
          if (wxExFindOtherFileName(*filename, list))
          {
            found = true;
          }
        break;
      }
    }

    if (event.GetId() == ID_LIST_VERSIONLIST && found)
    {
      list->SortColumn(_("Modified"), SORT_DESCENDING);
      list->DeleteDoubles();
    }
  }
  break;

  case ID_LIST_RUN_MAKE:
    wxExMake(m_Frame, wxExListItem(this, GetFirstSelected()).GetFileName());
  break;

  default: 
    wxFAIL;
    break;
  }
}

void wxExListViewWithFrame::RunItems(const wxExTool& tool)
{
  if ((tool.GetId() == ID_TOOL_REPORT_COUNT && GetType() == LIST_COUNT) ||
      (tool.GetId() == ID_TOOL_REPORT_KEYWORD && GetType() == LIST_KEYWORD) ||
      (tool.GetId() == ID_TOOL_REPORT_SQL && GetType() == LIST_SQL)
     )
  {
    return;
  }

  if (tool.IsFindType())
  {
    if (m_Frame->FindInSelectionDialog(tool.GetId()) == wxID_CANCEL)
    {
      return;
    }
  }

  if (!wxExTextFileWithListView::SetupTool(tool, m_Frame))
  {
    return;
  }

  wxExStatistics<long> stats;

  for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
  {
    stats += wxExRun(wxExListItem(this, i), tool).GetElements();
  }

  tool.Log(&stats);
}
