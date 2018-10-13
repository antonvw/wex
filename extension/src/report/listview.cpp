////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of class wex::history_listview
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

wex::history_listview::history_listview(const listview_data& data)
  : listview(data)
  , m_Frame(dynamic_cast<history_frame*>(wxTheApp->GetTopWindow()))
  , m_MenuFlags(data.Menu())
{
  if (GetData().Type() == LISTVIEW_HISTORY)
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
    listview* list = nullptr;
    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      listitem li(this, i);
      const wex::path* filename = &li.GetFileName();
      if (!filename->FileExists()) continue;
      switch (event.GetId())
      {
        case ID_LIST_COMPARE:
        {
          if (GetSelectedItemCount() == 1)
          {
            list = m_Frame->Activate(LISTVIEW_FILE);
            if (list == nullptr) return;
            const int main_selected = list->GetFirstSelected();
            comparefile(listitem(list, main_selected).GetFileName(), *filename);
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
            if (first) comparefile(path(file1), path(file2));
          }
        }
        break;
      }
    }}, ID_LIST_COMPARE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    make(listitem(this, GetFirstSelected()).GetFileName());}, ID_LIST_RUN_MAKE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const wex::tool& tool(event.GetId());
    if (tool.GetId() == ID_TOOL_REPORT_KEYWORD && GetData().Type() == LISTVIEW_KEYWORD) return;
    if (tool.IsFindType() && m_Frame->FindInFilesDialog(tool.GetId()) == wxID_CANCEL) return;
    if (!listview_stream::SetupTool(tool, m_Frame)) return;

#ifdef __WXMSW__    
    std::thread t([=] {
#endif

    statistics<int> stats;

    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      const listitem item(this, i);
      wxLogStatus(item.GetFileName().Path().string().c_str());
      if (item.GetFileName().FileExists())
      {
        listview_stream file(item.GetFileName(), tool);
        file.RunTool();
        stats += file.GetStatistics().GetElements();
      }
      else
      {
        tool_dir dir(tool, 
          item.GetFileName().Path().string(), 
          item.GetFileSpec());
        dir.FindFiles();
        stats += dir.GetStatistics().GetElements();
      }
    }
    log_status(tool.Info(&stats));
#ifdef __WXMSW__    
    });
    t.detach();
#endif
    }, ID_TOOL_LOWEST, ID_TOOL_HIGHEST);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::vector< path > files;
    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      files.emplace_back(listitem(this, i).GetFileName().Path());
    }
    vcs_execute(m_Frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, files);
    }, ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
}

void wex::history_listview::BuildPopupMenu(wex::menu& menu)
{
  bool exists = true, is_folder = false, is_make = false, read_only = false;

  if (GetSelectedItemCount() >= 1)
  {
    const listitem item(this, GetFirstSelected());

    exists = item.GetFileName().GetStat().IsOk();
    is_folder = item.GetFileName().DirExists();
    read_only = item.GetFileName().GetStat().IsReadOnly();
    is_make = item.GetFileName().GetLexer().GetScintillaLexer() == "makefile";
  }

  listview::BuildPopupMenu(menu);

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

    if ( GetData().Type() != LISTVIEW_FILE &&
        !wex::vcs().Use() &&
         exists && !is_folder)
    {
      if (auto* list = m_Frame->Activate(LISTVIEW_FILE);
        list != nullptr && list->GetSelectedItemCount() == 1)
      {
        listitem thislist(this, GetFirstSelected());
        const wxString current_file = thislist.GetFileName().Path().string();

        listitem otherlist(list, list->GetFirstSelected());

        if (const std::string with_file = otherlist.GetFileName().Path().string(); 
          current_file != with_file &&
            !wxConfigBase::Get()->Read(_("Comparator")).empty())
        {
          menu.AppendSeparator();
          menu.Append(ID_LIST_COMPARE,
            _("&Compare With") + " " + wxString(get_endoftext(with_file)));
        }
      }
    }
  }

  if (GetSelectedItemCount() >= 1)
  {
    if (exists && !is_folder)
    {
      if (vcs::DirExists(
        listitem(this, GetFirstSelected()).GetFileName()))
      {
        menu.AppendSeparator();
        menu.AppendVCS(listitem(this, GetFirstSelected()).GetFileName());
      }
    }

    // Finding in the LISTVIEW_FIND would result in recursive calls, do not add it.
    if ( exists &&
         GetData().Type() != LISTVIEW_FIND && (m_MenuFlags & LISTVIEW_MENU_REPORT_FIND))
    {
      menu.AppendSeparator();
      menu.Append(ID_TOOL_REPORT_FIND, 
        ellipsed(m_Frame->GetFindInCaption(ID_TOOL_REPORT_FIND)));

      if (!read_only)
      {
        menu.Append(ID_TOOL_REPLACE, 
          ellipsed(m_Frame->GetFindInCaption(ID_TOOL_REPLACE)));
      }
    }
  }

  if (GetSelectedItemCount() > 0 && exists && 
     (m_MenuFlags & LISTVIEW_MENU_TOOL) && !lexers::Get()->GetLexers().empty())
  {
    menu.AppendSeparator();
    menu.AppendTools();
  }
}

bool wex::history_listview::Destroy()	
{
  interruptable::Cancel();
  return listview::Destroy();
}

wex::listview_type wex::history_listview::GetTypeTool(
  const tool& tool)
{
  switch (tool.GetId())
  {
    case ID_TOOL_REPORT_FIND: return LISTVIEW_FIND;
    case ID_TOOL_REPORT_KEYWORD: return LISTVIEW_KEYWORD;
    default: return LISTVIEW_NONE;
  }
}
