////////////////////////////////////////////////////////////////////////////////
// Name:      frame.cpp
// Purpose:   Implementation of wxExFrameWithHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/tokenzr.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frd.h>
#include <wx/extension/listitem.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>

wxExFrameWithHistory::wxExFrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExManagedFrame(parent, id, title, maxFiles, style)
  , m_FiFDialog(NULL)
  , m_RiFDialog(NULL)
  , m_TextInFiles(_("In files"))
  , m_TextInFolder(_("In folder"))
  , m_TextRecursive(_("Recursive"))
  , m_FileHistoryList(NULL)
  , m_ProjectHistory(maxProjects, ID_RECENT_PROJECT_LOWEST, "RecentProject")
{
  // Take care of default value.
  if (!wxConfigBase::Get()->Exists(m_TextRecursive))
  {
    wxConfigBase::Get()->Write(m_TextRecursive, true); 
  }

  // This set determines what fields are placed on the Find Files dialogs
  // as a list of checkboxes.
  m_Info.insert(wxExFindReplaceData::Get()->GetTextMatchWholeWord());
  m_Info.insert(wxExFindReplaceData::Get()->GetTextMatchCase());
  m_Info.insert(wxExFindReplaceData::Get()->GetTextRegEx());
  
  Bind(wxEVT_IDLE, &wxExFrameWithHistory::OnIdle, this);
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_ProjectHistory.Save();
    event.Skip();});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_ProjectHistory.Clear();}, ID_CLEAR_PROJECTS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExListViewFile* project = GetProject();
    if (project != NULL)
    {
      project->FileSave();
    }}, ID_PROJECT_SAVE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!event.GetString().empty())
    {
      Grep(event.GetString());
    }
    else
    {
      if (m_FiFDialog == NULL)
      {
        CreateDialogs();
      }

      if  (GetSTC() != NULL && !GetSTC()->GetFindString().empty())
      {
        m_FiFDialog->Reload(); 
      }

      m_FiFDialog->Show(); 
    }}, ID_TOOL_REPORT_FIND);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_RiFDialog == NULL)
    {
      CreateDialogs();
    }
    if (GetSTC() != NULL && !GetSTC()->GetFindString().empty())
    {
      m_RiFDialog->Reload(); 
    }
    m_RiFDialog->Show();}, ID_TOOL_REPORT_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_ProjectHistory, event.GetId() - m_ProjectHistory.GetBaseId(), WIN_IS_PROJECT);},
    m_ProjectHistory.GetBaseId(), m_ProjectHistory.GetBaseId() + m_ProjectHistory.GetMaxFiles());
}

void wxExFrameWithHistory::CreateDialogs()
{
  std::set<wxString> t(m_Info);
  t.insert(m_TextRecursive);
  
  const std::vector<wxExConfigItem> f {
    wxExConfigItem(wxExFindReplaceData::Get()->GetTextFindWhat(), 
      ITEM_COMBOBOX, 
      wxEmptyString, 
      true),
    wxExConfigItem(m_TextInFiles, 
      ITEM_COMBOBOX, 
      wxEmptyString, 
      true),
    wxExConfigItem(m_TextInFolder, 
      ITEM_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      NewControlId()),
    wxExConfigItem(t)};
  
  m_FiFDialog = new wxExConfigDialog(this,
    f,
    _("Find In Files"),
    0,
    1,
    wxAPPLY | wxCANCEL,
    ID_FIND_IN_FILES);
    
  m_RiFDialog = new wxExConfigDialog(this,
    std::vector<wxExConfigItem> {f.at(0),
      wxExConfigItem(wxExFindReplaceData::Get()->GetTextReplaceWith(), 
      ITEM_COMBOBOX),
      f.at(1),
      f.at(2),
      wxExConfigItem(
        // Match whole word does not work with replace.
        std::set<wxString>{
        wxExFindReplaceData::Get()->GetTextMatchCase(),
        wxExFindReplaceData::Get()->GetTextRegEx(),
        m_TextRecursive})},
    _("Replace In Files"),
    0,
    1,
    wxAPPLY | wxCANCEL,
    ID_REPLACE_IN_FILES);
}

void wxExFrameWithHistory::FindInFiles(wxWindowID dialogid)
{
  const bool replace = (dialogid == ID_REPLACE_IN_FILES);
  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool, this))
  {
    return;
  }

  wxLogStatus(GetFindReplaceInfoText(replace));
    
  int flags = wxDIR_FILES | wxDIR_HIDDEN;
  
  if (wxConfigBase::Get()->ReadBool(m_TextRecursive, true)) 
  {
    flags |= wxDIR_DIRS;
  }

  Unbind(wxEVT_IDLE, &wxExFrameWithHistory::OnIdle, this);

#ifndef __WXGTK__    
  std::thread t([=] {
#endif
    wxExDirTool dir(
      tool,
      wxExConfigFirstOf(m_TextInFolder),
      wxExConfigFirstOf(m_TextInFiles),
      flags);

    dir.FindFiles();

    wxLogStatus(tool.Info(&dir.GetStatistics().GetElements()));
    
    Bind(wxEVT_IDLE, &wxExFrameWithHistory::OnIdle, this);
#ifndef __WXGTK__    
    });
  t.detach();
#endif
}

bool wxExFrameWithHistory::FindInFiles(
  const std::vector< wxString > & files,
  int id,
  bool show_dialog,
  wxExListView* report)
{
  const wxExFileName filename(files[0]);
  const wxExTool tool(id);
  
  if (show_dialog && FindInFilesDialog(
    tool.GetId(),
    filename.DirExists() && !filename.FileExists()) == wxID_CANCEL)
  {
    return false;
  }
  
  if (!wxExTextFileWithListView::SetupTool(tool, this, report))
  {
    return false;
  }
  
#ifndef __WXGTK__    
  std::thread t([=] {
#endif
    wxExStatistics<int> stats;
    
    for (const auto& it : files)
    {
      const wxExFileName fn(it);
      
      if (fn.FileExists())
      {
        wxExTextFileWithListView file(fn, tool);
        file.RunTool();
        stats += file.GetStatistics().GetElements();
      }
      else
      {
        wxExDirTool dir(
          tool, 
          fn.GetFullPath(), 
          wxExConfigFirstOf(m_TextInFiles));
          
        dir.FindFiles();
        stats += dir.GetStatistics().GetElements();
      }
    }
    
    wxLogStatus(tool.Info(&stats));
  
#ifndef __WXGTK__    
    });
  t.detach();
#endif

  return true;
}

int wxExFrameWithHistory::FindInFilesDialog(
  int id,
  bool add_in_files)
{
  if (GetSTC() != NULL)
  {
    GetSTC()->GetFindString();
  }

  if (wxExConfigDialog(this,
    std::vector<wxExConfigItem> {
      wxExConfigItem(
        wxExFindReplaceData::Get()->GetTextFindWhat(), 
        ITEM_COMBOBOX, 
        wxEmptyString, 
        true),
      (add_in_files ? wxExConfigItem(
        m_TextInFiles, 
        ITEM_COMBOBOX, 
        wxEmptyString, 
        true) : wxExConfigItem()),
      (id == ID_TOOL_REPORT_REPLACE ? wxExConfigItem(
        wxExFindReplaceData::Get()->GetTextReplaceWith(), 
        ITEM_COMBOBOX): wxExConfigItem()),
      wxExConfigItem(m_Info)},
    GetFindInCaption(id)).ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  wxLogStatus(GetFindReplaceInfoText(id == ID_TOOL_REPORT_REPLACE));
        
  return wxID_OK;
}

const wxString wxExFrameWithHistory::GetFindInCaption(int id) const
{
  return (id == ID_TOOL_REPORT_REPLACE ?
    _("Replace In Selection"):
    _("Find In Selection"));
}

const wxString wxExFrameWithHistory::GetFindReplaceInfoText(bool replace) const
{
  wxString log;
  
  // Printing a % in wxLogStatus gives assert
  if (
    !wxExFindReplaceData::Get()->GetFindString().Contains("%") &&
    !wxExFindReplaceData::Get()->GetReplaceString().Contains("%"))
  {
    log = _("Searching for") + ": " + wxExFindReplaceData::Get()->GetFindString();

    if (replace)
    {
      log += " " + _("replacing with") + ": " + wxExFindReplaceData::Get()->GetReplaceString();
    }
  }

  return log;
}

bool wxExFrameWithHistory::Grep(const wxString& arg)
{
  wxString arg1, arg2;
  int arg3;
  if (wxExCmdLineParser(arg, 
    wxExCmdLineParser::CmdSwitches {
      {{"r", "recursive"}, {0, [&](bool on) {arg3 = wxDIR_FILES | (on ? wxDIR_DIRS: 0);}}}},
    wxExCmdLineParser::CmdOptions(),
    wxExCmdLineParser::CmdParams {
      {"match", {0, [&](std::vector<wxString> & v) {wxExFindReplaceData::Get()->SetFindString(v[0]);}}},
      {"folder", {0, [&](std::vector<wxString> & v) {arg1 = v[1];}}},
      {"extension", {0, [&](std::vector<wxString> & v) {arg2 = v[2];}}}}).Parse() != 0)
  {
    return false;
  }
  
  const wxExTool tool(ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool, this))
  {
    wxLogStatus("setup failed");
    return false;
  }
  
  auto* stc = GetSTC();
  if (stc != NULL)
    wxSetWorkingDirectory(stc->GetFileName().GetPath());
  wxExFindReplaceData::Get()->SetUseRegEx(true);
  wxLogStatus(GetFindReplaceInfoText());
  
  Unbind(wxEVT_IDLE, &wxExFrameWithHistory::OnIdle, this);
    
#ifndef __WXGTK__    
  std::thread t([=]{
#endif
    wxExDirTool dir(tool, arg1, arg2, arg3);
    dir.FindFiles();
    wxLogStatus(tool.Info(&dir.GetStatistics().GetElements()));
    
    Bind(wxEVT_IDLE, &wxExFrameWithHistory::OnIdle, this);
#ifndef __WXGTK__    
    });
  t.detach();
#endif
    
  return true;
}

void wxExFrameWithHistory::OnCommandItemDialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (wxExInterruptable::Cancel())
      {
        wxLogStatus(_("Cancelled"));
      }
      break;

    case wxID_OK:
    case wxID_APPLY:
      switch (dialogid)
      {
        case wxID_ADD:
          if (GetProject() != NULL)
          {
            int flags = 0;
          
            if (wxConfigBase::Get()->ReadBool(GetProject()->GetTextAddFiles(), true)) 
            {
              flags |= wxDIR_FILES;
            }
          
            if (wxConfigBase::Get()->ReadBool(GetProject()->GetTextAddRecursive(), true)) 
            {
              flags |= wxDIR_DIRS;
            }

            GetProject()->AddItems(
              wxExConfigFirstOf(GetProject()->GetTextInFolder()),
              wxExConfigFirstOf(GetProject()->GetTextAddWhat()),
              flags);
          }
          break;

        case ID_FIND_IN_FILES:
        case ID_REPLACE_IN_FILES:
          FindInFiles(dialogid);
          break;

        default: wxFAIL;
      }
      break;

    default: wxFAIL;
  }
}

void wxExFrameWithHistory::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  const wxString title(GetTitle());
  
  if (title.empty())
  {
    return;
  }
  
  auto* stc = GetSTC();
  auto* project = GetProject();

  const wxUniChar indicator('*');

  if ((project != NULL && project->GetContentsChanged()) ||
       // using GetContentsChanged gives assert in vcs dialog
      (stc != NULL && stc->GetModify() && stc->AllowChangeIndicator()))
  {
    // Project or editor changed, add indicator if not yet done.
    if (title.Last() != indicator)
    {
      SetTitle(title + " " + indicator);
    }
  }
  else
  {
    // Project or editor not changed, remove indicator if not yet done.
    if (title.Last() == indicator && title.size() > 2)
    {
      SetTitle(title.substr(0, title.length() - 2));
    }
  }
}

void wxExFrameWithHistory::SetRecentFile(const wxString& file)
{
  wxExManagedFrame::SetRecentFile(file);
  
  if (m_FileHistoryList != NULL)
  {
    wxExListItem item(m_FileHistoryList, file);
    item.Insert((long)0);

    if (m_FileHistoryList->GetItemCount() > 1)
    {
      for (int i = m_FileHistoryList->GetItemCount() - 1; i >= 1 ; i--)
      {
        wxExListItem item(m_FileHistoryList, i);

        if (item.GetFileName().GetFullPath() == file)
        {
          item.Delete();
        }
      }
    }
  }
}

void wxExFrameWithHistory::UseFileHistoryList(wxExListView* list)
{
  m_FileHistoryList = list;
  m_FileHistoryList->Hide();

  // Add all (existing) items from FileHistory.
  for (size_t i = 0; i < GetFileHistory().GetCount(); i++)
  {
    wxExListItem item(
      m_FileHistoryList, 
      GetFileHistory().GetHistoryFile(i));

    if (item.GetFileName().GetStat().IsOk())
    {
      item.Insert();
    }
  }
}
