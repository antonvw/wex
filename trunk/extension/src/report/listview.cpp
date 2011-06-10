/******************************************************************************\
* File:          listview.cpp
* Purpose:       Implementation of class 'wxExListViewStandard' 
                 and 'wxExListViewWithFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcs.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

BEGIN_EVENT_TABLE(wxExListViewStandard, wxExListView)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, wxExListViewStandard::OnList)
  EVT_MENU(wxID_CUT, wxExListViewStandard::OnCommand)
  EVT_MENU(wxID_CLEAR, wxExListViewStandard::OnCommand)
  EVT_MENU(wxID_DELETE, wxExListViewStandard::OnCommand)
  EVT_MENU(wxID_PASTE, wxExListViewStandard::OnCommand)
  EVT_MENU(ID_LIST_SEND_ITEM, wxExListViewStandard::OnCommand)
END_EVENT_TABLE()

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
class RBSFile: public wxFile
{
public:
  RBSFile(wxExListView* listview);
  void GenerateDialog();
private:
  void Body(
    const wxString& filename,
    const wxString& source,
    const wxString& destination);
  void Footer();
  void GenerateTransmit(const wxString& text);
  void GenerateWaitFor(const wxString& text);
  void Header();
  bool Substitute(
    wxString& text,
    const wxString& pattern,
    const wxString& new_pattern,
    const bool is_required);
  wxExListView* m_ListView;
  wxString m_Prompt;
};
#endif // wxExUSE_RBS
#endif // __WXMSW__

wxExListViewStandard::wxExListViewStandard(wxWindow* parent,
  ListType type,
  wxWindowID id,
  const wxExLexer* lexer,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString &name)
  : wxExListView(
      parent, 
      id, 
      pos, 
      size, 
      style, 
      IMAGE_FILE_ICON, 
      validator, 
      name)
  , m_Type(type)
{
  Initialize(lexer);
}

void wxExListViewStandard::BuildPopupMenu(wxExMenu& menu)
{
  long style = 0;

  if (GetSelectedItemCount() > 0) style |= wxExMenu::MENU_IS_SELECTED;
  if (GetItemCount() == 0) style |= wxExMenu::MENU_IS_EMPTY;
  if (GetSelectedItemCount() == 0 && GetItemCount() > 0) style |= wxExMenu::MENU_ALLOW_CLEAR;

  menu.SetStyle(style);

  wxExListView::BuildPopupMenu(menu);

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
  if (GetSelectedItemCount() >= 1)
  {
    bool exists = true;
    bool is_folder = false;

    if (GetSelectedItemCount() == 1)
    {
      const wxExListItem item(this, GetFirstSelected());

      is_folder = item.GetFileName().DirExists();
      exists = item.GetFileName().GetStat().IsOk();
    }

    if (exists && !is_folder)
    {
      menu.AppendSeparator();
      menu.Append(ID_LIST_SEND_ITEM, wxExEllipsed(_("&Build RBS File")));
    }
  }
#endif
#endif
}

void wxExListViewStandard::DeleteDoubles()
{
  wxDateTime mtime((time_t)0);
  wxString name;
  const auto itemcount = GetItemCount();

  for (auto i = itemcount - 1; i >= 0; i--)
  {
    wxExListItem item(this, i);

    // Delete this element if it has the same mtime
    // and the same name as the previous one.
    if (mtime == item.GetFileName().GetStat().st_mtime &&
        name == GetItemText(i, _("File Name")))
    {
      DeleteItem(i);
    }
    else
    {
      mtime = item.GetFileName().GetStat().st_mtime;
      name = GetItemText(i, _("File Name"));
    }
  }

  if (itemcount != GetItemCount())
  {
    ItemsUpdate();
  }
}

const wxString wxExListViewStandard::GetTypeDescription(ListType type)
{
  wxString value;

  switch (type)
  {
  case LIST_COUNT: value = _("File Count"); break;
  case LIST_FIND: value = _("Find Results"); break;
  case LIST_HISTORY: value = _("History"); break;
  case LIST_KEYWORD: value = _("Keywords"); break;
  case LIST_PROCESS: value = _("Process Output"); break;
  case LIST_FILE: value = _("File"); break;
  case LIST_REPLACE: value = _("Replace Results"); break;
  case LIST_REVISION: value = _("Revisions"); break;
  case LIST_SQL: value = _("SQL Queries"); break;
  case LIST_VERSION: value = _("Version List"); break;
  default: wxFAIL;
  }

  return value;
}

wxExListViewStandard::ListType wxExListViewStandard::GetTypeTool(
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

void wxExListViewStandard::Initialize(const wxExLexer* lexer)
{
  SetName(GetTypeDescription());

  if (m_Type == LIST_KEYWORD)
  {
    if (lexer == NULL)
    {
      wxFAIL;
      return;
    }

    SetName(GetName() + " " + lexer->GetScintillaLexer());
  }

  const int col_line_width = 250;

  if (m_Type != LIST_PROCESS)
  {
    InsertColumn(wxExColumn(_("File Name"), wxExColumn::COL_STRING));
  }

  switch (m_Type)
  {
  case LIST_COUNT:
    // See wxExTextFileWithListView::Report, 
    // the order in which columns are set should be the same there.
    InsertColumn(wxExColumn(_("Lines")));
    InsertColumn(wxExColumn(_("Lines Of Code")));
    InsertColumn(wxExColumn(_("Empty Lines")));
    InsertColumn(wxExColumn(_("Words Of Code")));
    InsertColumn(wxExColumn(_("Comments")));
    InsertColumn(wxExColumn(_("Comment Size")));
  break;
  case LIST_FIND:
  case LIST_REPLACE:
    InsertColumn(wxExColumn(_("Line"), wxExColumn::COL_STRING, col_line_width));
    InsertColumn(wxExColumn(_("Match"), wxExColumn::COL_STRING));
    InsertColumn(wxExColumn(_("Line No")));
  break;
  case LIST_KEYWORD:
    for (
      auto it = lexer->GetKeywords().begin();
      it != lexer->GetKeywords().end();
      ++it)
    {
      InsertColumn(wxExColumn(*it));
    }

    InsertColumn(wxExColumn(_("Keywords")));
  break;
  case LIST_PROCESS:
    InsertColumn(wxExColumn(_("Line"), wxExColumn::COL_STRING, col_line_width));
    InsertColumn(wxExColumn(_("Line No")));
    InsertColumn(wxExColumn(_("File Name"), wxExColumn::COL_STRING));
  break;
  case LIST_REVISION:
    InsertColumn(wxExColumn(_("Revision Comment"), wxExColumn::COL_STRING, 400));
    InsertColumn(wxExColumn(_("Date"), wxExColumn::COL_DATE));
    InsertColumn(wxExColumn(_("Initials"), wxExColumn::COL_STRING));
    InsertColumn(wxExColumn(_("Line No")));
    InsertColumn(wxExColumn(_("Revision"), wxExColumn::COL_STRING));
  break;
  case LIST_SQL:
    InsertColumn(wxExColumn(_("Run Time"), wxExColumn::COL_DATE));
    InsertColumn(wxExColumn(_("Query"), wxExColumn::COL_STRING, 400));
    InsertColumn(wxExColumn(_("Line No")));
  break;
  default: break; // to prevent warnings
  }

  if (m_Type == LIST_REPLACE)
  {
    InsertColumn(wxExColumn(_("Replaced")));
  }

  InsertColumn(wxExColumn(_("Modified"), wxExColumn::COL_DATE));
  InsertColumn(wxExColumn(_("In Folder"), wxExColumn::COL_STRING, 175));
  InsertColumn(wxExColumn(_("Type"), wxExColumn::COL_STRING));
  InsertColumn(wxExColumn(_("Size")));
}

bool wxExListViewStandard::ItemFromText(const wxString& text)
{
  if (text.empty())
  {
    return false;
  }

  bool modified = false;
  wxStringTokenizer tk(text, wxTextFile::GetEOL());

  while (tk.HasMoreTokens())
  {
    modified = true;

    wxStringTokenizer tkz(tk.GetNextToken(), GetFieldSeparator());
    if (tkz.HasMoreTokens())
    {
      const wxString value = tkz.GetNextToken();
      wxFileName fn(value);

      if (fn.FileExists())
      {
        wxExListItem item(this, fn);
        item.Insert();

        // And try to set the rest of the columns 
        // (that are not already set by inserting).
        int col = 1;
        while (tkz.HasMoreTokens() && col < GetColumnCount() - 1)
        {
          const wxString value = tkz.GetNextToken();

          if (col != FindColumn(_("Type")) &&
              col != FindColumn(_("In Folder")) &&
              col != FindColumn(_("Size")) &&
              col != FindColumn(_("Modified")))
          {
            item.SetItem(col, value);
          }

          col++;
        }
      }
      else
      {
        // Now we need only the first column (containing findfiles). If more
        // columns are present, these are ignored.
        const wxString findfiles =
          (tkz.HasMoreTokens() ? tkz.GetNextToken(): tkz.GetString());

        wxExListItem(this, value, findfiles).Insert();
      }
    }
    else
    {
      wxExListItem(this, text).Insert();
    }
  }

  return modified;
}

const wxString wxExListViewStandard::ItemToText(long item_number) const
{
  wxExListItem item(
    const_cast< wxExListViewStandard * >(this), item_number);

  wxString text = (item.GetFileName().GetStat().IsOk() ? 
    item.GetFileName().GetFullPath(): 
    item.GetFileName().GetFullName());

  if (item.GetFileName().DirExists())
  {
    text += GetFieldSeparator() + GetItemText(item_number, _("Type"));
  }

  if (m_Type != LIST_FILE)
  {
    text += GetFieldSeparator() + wxExListView::ItemToText(item_number);
  }

  return text;
}

void wxExListViewStandard::ItemsUpdate()
{
  for (auto i = 0; i < GetItemCount(); i++)
  {
    wxExListItem(this, i).Update();
  }
}

void wxExListViewStandard::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CLEAR:
    case wxID_CUT:
    case wxID_DELETE:
    case wxID_PASTE:
      if (m_Type != LIST_HISTORY)
      {
        event.Skip();
      }
    break;

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
  case ID_LIST_SEND_ITEM:
    RBSFile(this).GenerateDialog();
    break;
#endif
#endif

  default: 
    wxFAIL;
    break;
  }
}

void wxExListViewStandard::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED)
  {
    if (GetSelectedItemCount() == 1)
    {
      wxExLogStatus(wxExListItem(this, event.GetIndex()).GetFileName(), 
        STAT_FULLPATH);
    }

    event.Skip();
  }
  else
  {
    wxFAIL;
  }
}

BEGIN_EVENT_TABLE(wxExListViewWithFrame, wxExListViewStandard)
  EVT_LIST_ITEM_ACTIVATED(wxID_ANY, wxExListViewWithFrame::OnList)
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
  : wxExListViewStandard(
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

  wxExListViewStandard::BuildPopupMenu(menu);

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

void wxExListViewWithFrame::ItemActivated(long item_number)
{
  wxASSERT(item_number >= 0);
 
  // Cannot be const because of SetItem later on.
  wxExListItem item(this, item_number);

  if (!item.GetFileName().FileExists() &&
       item.GetFileName().DirExists())
  {
    wxTextEntryDialog dlg(this,
      _("Input") + ":",
      _("Folder Type"),
      GetItemText(item_number, _("Type")));

    if (dlg.ShowModal() == wxID_OK)
    {
      item.SetItem(_("Type"), dlg.GetValue());
    }
  }
  else if (item.GetFileName().FileExists())
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

    wxExListViewStandard* list = NULL;

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

void wxExListViewWithFrame::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_ACTIVATED)
  {
    ItemActivated(event.GetIndex());
  }
  else
  {
    wxFAIL;
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
    stats += wxExListItem(this, i).Run(tool).GetElements();
  }

  tool.Log(&stats);
}

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
RBSFile::RBSFile(wxExListView* listview)
  : wxFile()
  , m_ListView(listview)
  , m_Prompt(wxConfigBase::Get()->Read("RBS/Prompt", ">"))
{
}

void RBSFile::Body(
  const wxString& filename,
  const wxString& source,
  const wxString& destination)
{
  GenerateTransmit("SET DEF [" + destination + "]");
  GenerateTransmit("; *** Sending: " + filename + " ***");
  GenerateWaitFor(m_Prompt);
  GenerateTransmit("KERMIT RECEIVE");
  Write(".KermitSendFile \"" + source + wxFILE_SEP_PATH + filename + "\",\"" + filename + ("\",rcASCII\n"));

  GenerateTransmit(wxEmptyString);
  GenerateWaitFor(m_Prompt);
  GenerateTransmit("; *** Done: " + filename + " ***");
}

void RBSFile::Footer()
{
  Write("End With\n");
  Write("End Sub\n");
}

void RBSFile::GenerateDialog()
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem(_("RBS File"), CONFIG_FILEPICKERCTRL, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("RBS Pattern"), CONFIG_DIRPICKERCTRL));
  wxExConfigDialog dlg(NULL, v, _("Build RBS File"));
  if (dlg.ShowModal() == wxID_CANCEL) return;

  const wxString script = wxConfigBase::Get()->Read(_("RBS File"));

  if (!Open(script, wxFile::write))
  {
    return;
  }

  wxBusyCursor wait;

  Header();

  const wxString rsx_pattern = wxConfigBase::Get()->Read(_("RBS Pattern")) + wxFILE_SEP_PATH;
  for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
  {
    wxExListItem li(m_ListView, i);
    const wxFileName* filename = &li.GetFileName();
    if (!wxFileName::DirExists(filename->GetFullPath()))
    {
      const wxString source = filename->GetPath();
      wxString destination = source, pattern, with;
      if (source.find(rsx_pattern) != wxString::npos)
      {
        pattern = rsx_pattern;
        with = wxConfigBase::Get()->Read("RBS/With");
      }
      else
      {
        wxLogError("Cannot find: %s inside: %s", rsx_pattern.c_str(), source.c_str());
        return;
      }

      if (!Substitute(destination, pattern, with, true)) return;
      Substitute(destination, wxFILE_SEP_PATH, ",", false);
      Body(filename->GetFullName(), source, destination);
    }
  }

  Footer();
  Close();

  wxLogStatus("RBS " + _("File") + ": " + script + " " + _("generated"));
}

void RBSFile::GenerateTransmit(const wxString& text)
{
  if (text.empty()) Write(".Transmit Chr$(13)\n");
  else              Write(".Transmit \"" + text + "\" & Chr$(13)\n");
}

void RBSFile::GenerateWaitFor(const wxString& text)
{
  const wxString pdp_11_spec = "Chr$(10) & ";
  Write(".WaitForString " + pdp_11_spec + "\"" + text + "\", 0, rcAllowKeyStrokes\n");
  Write(".Wait 1, rcAllowKeyStrokes\n");
}

void RBSFile::Header()
{
  wxASSERT(wxTheApp != NULL);
  Write("' Script generated by: " + wxTheApp->GetAppDisplayName() + ": " + wxDateTime::Now().Format() + "\n" +
        "' Do not modify this file, all changes will be lost!\n\n" +
        "Option Explicit\n" +
        "Sub Main\n\n" +
        "With Application\n\n");
}

bool RBSFile::Substitute(
  wxString& text,
  const wxString& pattern,
  const wxString& new_pattern,
  const bool is_required)
{
  size_t pos_pattern;
  if ((pos_pattern = text.find(pattern)) == wxString::npos)
  {
    if (is_required)
    {
      wxLogError("Cannot find pattern: " + pattern + " in: " + text);
    }

    return false;
  }

  text = text.substr(0, pos_pattern) + new_pattern + text.substr(pos_pattern + pattern.length());

  return true;
}
#endif // wxExUSE_RBS
#endif // __WXMSW__
