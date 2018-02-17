////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of class wxExListViewWithFrame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/interruptable.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/stream.h>

wxExListViewWithFrame::wxExListViewWithFrame(const wxExListViewData& data)
  : wxExListView(data)
  , m_Frame(dynamic_cast<wxExFrameWithHistory*>(wxTheApp->GetTopWindow()))
  , m_MenuFlags(data.Menu())
{
  if (GetData().Type() == LIST_HISTORY)
  {
    m_Frame->UseFileHistoryList(this);
  }

  wxAcceleratorEntry entries[5];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[1].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[2].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[3].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);
  entries[4].Set(wxACCEL_CTRL, 'M', ID_LIST_COMPARE);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    bool first = true;
    wxString file1,file2;
    wxExListView* list = nullptr;
    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      wxExListItem li(this, i);
      const wxExPath* filename = &li.GetFileName();
      if (!filename->FileExists()) continue;
      switch (event.GetId())
      {
        case ID_LIST_COMPARE:
        {
          if (GetSelectedItemCount() == 1)
          {
            list = m_Frame->Activate(LIST_FILE);
            if (list == nullptr) return;
            const int main_selected = list->GetFirstSelected();
            wxExCompareFile(wxExListItem(list, main_selected).GetFileName(), *filename);
          }
          else
          {
            if (first)
            {
              first = false;
              file1 = filename->Path().string();
            }
            else
            {
              first = true;
              file2 = filename->Path().string();
            }
            if (first) wxExCompareFile(wxExPath(file1), wxExPath(file2));
          }
        }
        break;
      }
    }}, ID_LIST_COMPARE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExMake(wxExListItem(this, GetFirstSelected()).GetFileName());}, ID_LIST_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const wxExTool& tool(event.GetId());
    if (tool.GetId() == ID_TOOL_REPORT_KEYWORD && GetData().Type() == LIST_KEYWORD) return;
    if (tool.IsFindType() && m_Frame->FindInFilesDialog(tool.GetId()) == wxID_CANCEL) return;
    if (!wxExStreamToListView::SetupTool(tool, m_Frame)) return;

#ifdef __WXMSW__    
    std::thread t([=] {
#endif

    wxExStatistics<int> stats;

    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      const wxExListItem item(this, i);
      wxLogStatus(item.GetFileName().Path().string().c_str());
      if (item.GetFileName().FileExists())
      {
        wxExStreamToListView file(item.GetFileName(), tool);
        file.RunTool();
        stats += file.GetStatistics().GetElements();
      }
      else
      {
        wxExDirTool dir(tool, 
          item.GetFileName().Path().string(), 
          item.GetFileSpec());
        dir.FindFiles();
        stats += dir.GetStatistics().GetElements();
      }
    }
    wxExLogStatus(tool.Info(&stats));
#ifdef __WXMSW__    
    });
    t.detach();
#endif
    }, ID_TOOL_LOWEST, ID_TOOL_HIGHEST);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::vector< wxExPath > files;
    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      files.emplace_back(wxExListItem(this, i).GetFileName().Path());
    }
    wxExVCSExecute(m_Frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, files);
    }, ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
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
    is_folder = item.GetFileName().DirExists();
    read_only = item.GetFileName().GetStat().IsReadOnly();
    is_make = item.GetFileName().GetLexer().GetScintillaLexer() == "makefile";
  }

  wxExListView::BuildPopupMenu(menu);

  if (GetSelectedItemCount() > 1 && exists &&
     !wxConfigBase::Get()->Read(_("Comparator")).empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_LIST_COMPARE, _("C&ompare") + "\tCtrl+M");
  }

  if (GetSelectedItemCount() == 1)
  {
    if (is_make)
    {
      menu.AppendSeparator();
      menu.Append(ID_LIST_RUN_MAKE, _("&Make"));
    }

    if ( GetData().Type() != LIST_FILE &&
        !wxExVCS().Use() &&
         exists && !is_folder)
    {
      auto* list = m_Frame->Activate(LIST_FILE);

      if (list != nullptr && list->GetSelectedItemCount() == 1)
      {
        wxExListItem thislist(this, GetFirstSelected());
        const wxString current_file = thislist.GetFileName().Path().string();

        wxExListItem otherlist(list, list->GetFirstSelected());
        const std::string with_file = otherlist.GetFileName().Path().string();

        if (current_file != with_file &&
            !wxConfigBase::Get()->Read(_("Comparator")).empty())
        {
          menu.AppendSeparator();
          menu.Append(ID_LIST_COMPARE,
            _("&Compare With") + " " + wxString(wxExGetEndOfText(with_file)));
        }
      }
    }
  }

  if (GetSelectedItemCount() >= 1)
  {
    if (exists && !is_folder)
    {
      if (wxExVCS::DirExists(
        wxExListItem(this, GetFirstSelected()).GetFileName()))
      {
        menu.AppendSeparator();
        menu.AppendVCS(wxExListItem(this, GetFirstSelected()).GetFileName());
      }
    }

    // Finding in the LIST_FIND would result in recursive calls, do not add it.
    if ( exists &&
         GetData().Type() != LIST_FIND && (m_MenuFlags & LIST_MENU_REPORT_FIND))
    {
      menu.AppendSeparator();
      menu.Append(ID_TOOL_REPORT_FIND, 
        wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

      if (!read_only)
      {
        menu.Append(ID_TOOL_REPLACE, 
          wxExEllipsed(m_Frame->GetFindInCaption(ID_TOOL_REPLACE)));
      }
    }
  }

  if (GetSelectedItemCount() > 0 && exists && 
     (m_MenuFlags & LIST_MENU_TOOL) && !wxExLexers::Get()->GetLexers().empty())
  {
    menu.AppendSeparator();
    menu.AppendTools();
  }
}

bool wxExListViewWithFrame::Destroy()	
{
  wxExInterruptable::Cancel();
  return wxExListView::Destroy();
}

wxExListType wxExListViewWithFrame::GetTypeTool(
  const wxExTool& tool)
{
  switch (tool.GetId())
  {
    case ID_TOOL_REPORT_FIND: return LIST_FIND;
    case ID_TOOL_REPORT_KEYWORD: return LIST_KEYWORD;
    default: return LIST_NONE;
  }
}
