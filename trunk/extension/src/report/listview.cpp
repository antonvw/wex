/******************************************************************************\
* File:          listview.cpp
* Purpose:       Implementation of class 'wxExListViewFile' and support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/svn.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/util.h>

#if wxUSE_DRAG_AND_DROP
class ListViewDropTarget : public wxFileDropTarget
{
public:
  ListViewDropTarget(wxExListViewFile* owner) {m_Owner = owner;}
private:
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
  wxExListViewFile* m_Owner;
};
#endif

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
class RBSFile : public wxExFile
{
public:
  RBSFile(wxExListViewFile* listview);
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
  wxExListViewFile* m_Owner;
  wxString m_Prompt;
};
#endif // __WXMSW__
#endif // wxExUSE_RBS

BEGIN_EVENT_TABLE(wxExListViewFile, wxExListView)
  EVT_IDLE(wxExListViewFile::OnIdle)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, wxExListViewFile::OnList)
  EVT_MENU(wxID_ADD, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_CUT, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_CLEAR, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_DELETE, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_PASTE, wxExListViewFile::OnCommand)
  EVT_MENU(ID_LIST_SEND_ITEM, wxExListViewFile::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_SVN_LOWEST, ID_EDIT_SVN_HIGHEST, wxExListViewFile::OnCommand)
  EVT_LEFT_DOWN(wxExListViewFile::OnMouse)
END_EVENT_TABLE()

wxExListViewFile::wxExListViewFile(wxWindow* parent,
  ListType type,
  wxWindowID id,
  long menu_flags,
  const wxExLexer* lexer,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString &name)
  : wxExListView(parent, id, pos, size, style, IMAGE_FILE_ICON, validator, name)
  , wxExFile()
  , m_ContentsChanged(false)
  , m_ItemUpdated(false)
  , m_ItemNumber(0)
  , m_MenuFlags(menu_flags)
  , m_Type(type)
{
  Initialize(lexer);
}

wxExListViewFile::wxExListViewFile(wxWindow* parent,
  const wxString& file,
  wxWindowID id,
  long menu_flags,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString& name)
  : wxExListView(parent, id, pos, size, style, IMAGE_FILE_ICON, validator, name)
  , wxExFile()
  , m_ContentsChanged(false)
  , m_ItemUpdated(false)
  , m_ItemNumber(0)
  , m_MenuFlags(menu_flags)
  , m_Type(LIST_PROJECT)
{
  Initialize(NULL);
  wxExFile::FileLoad(file);
}

void wxExListViewFile::AddItems()
{
  std::vector<wxExConfigItem> v;
  v.push_back(wxExConfigItem(_("Add what"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(wxExConfigItem());
  std::set<wxString> set;
  set.insert(_("Add files"));
  set.insert(_("Add folders"));
  set.insert(_("Recursive"));
  v.push_back(wxExConfigItem(set));

  wxExConfigDialog dlg(this,
    v,
    _("Add Files"));

  // Force at least one of the checkboxes to be checked.
  dlg.ForceCheckBoxChecked(_("Add"));

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return;
  }

  int flags = 0;
  if (wxConfigBase::Get()->ReadBool(_("Add files"), true)) flags |= wxDIR_FILES;
  if (wxConfigBase::Get()->ReadBool(_("Recursive"), false)) flags |= wxDIR_DIRS;

  wxExDirWithListView dir(
    this,
    wxExConfigFirstOf(_("In folder")),
    wxExConfigFirstOf(_("Add what")),
    flags);

  const int old_count = GetItemCount();
  dir.FindFiles();
  const int new_count = GetItemCount();

  if (new_count - old_count > 0)
  {
    m_ContentsChanged = true;

    if (wxConfigBase::Get()->ReadBool("List/SortSync", true) && 
        m_Type == LIST_PROJECT)
    {
      SortColumn(_("Modified"), SORT_KEEP);
    }
  }

#if wxUSE_STATUSBAR
  const wxString text = 
    _("Added") + wxString::Format(" %d ", new_count - old_count) + _("file(s)");

  wxExFrame::StatusText(text);
#endif
}

void wxExListViewFile::AfterSorting()
{
  // Only if we are a project list and not sort syncing, 
  // set contents changed.
  if ( m_Type == LIST_PROJECT &&
      !wxConfigBase::Get()->ReadBool("List/SortSync", true))
  {
    m_ContentsChanged = true;
  }
}

void wxExListViewFile::BuildPopupMenu(wxExMenu& menu)
{
  long style;

  if (m_Type == LIST_PROJECT)
  {
    // This contains the CAN_PASTE flag, only for these types.
    style = wxExMenu::MENU_DEFAULT;
  }
  else
  {
    style = 0;
  }

  if ((GetFileName().FileExists() && GetFileName().GetStat().IsReadOnly()) ||
       m_Type == LIST_HISTORY)
  {
    style |= wxExMenu::MENU_IS_READ_ONLY;
  }

  if (GetSelectedItemCount() > 0) style |= wxExMenu::MENU_IS_SELECTED;
  if (GetItemCount() == 0) style |= wxExMenu::MENU_IS_EMPTY;

  if (GetSelectedItemCount() == 0 && 
      GetItemCount() > 0 &&
      m_Type != LIST_HISTORY) 
  {
    style |= wxExMenu::MENU_ALLOW_CLEAR;
  }

  menu.SetStyle(style);

  bool exists = true;
  bool is_folder = false;

  if (GetSelectedItemCount() == 1)
  {
    const wxExListItemWithFileName item(this, GetFirstSelected());

    is_folder = wxDirExists(item.GetFileName().GetFullPath());
    exists = item.GetFileName().GetStat().IsOk();
  }

  if (GetSelectedItemCount() >= 1)
  {
    wxExListView::BuildPopupMenu(menu);

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
    if (exists && !is_folder)
    {
      menu.AppendSeparator();
      menu.Append(ID_LIST_SEND_ITEM, wxExEllipsed(_("&Build RBS File")));
    }
#endif
#endif
  }
  else
  {
    if (m_Type == LIST_PROJECT)
    {
      if (!GetFileName().IsOk() ||
          !GetFileName().FileExists() ||
          (GetFileName().FileExists() && !GetFileName().GetStat().IsReadOnly()))
      {
        menu.AppendSeparator();
        menu.Append(wxID_ADD);
      }
    }

    menu.AppendSeparator();
    wxExListView::BuildPopupMenu(menu);
  }
}

void wxExListViewFile::FileNew(const wxExFileName& filename)
{
  wxExFile::FileNew(filename);
  EditClearAll();
}

void wxExListViewFile::DoFileLoad(bool synced)
{
  EditClearAll();

  const wxCharBuffer& buffer = Read();

  wxStringTokenizer tkz(buffer.data(), wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    ItemFromText(tkz.GetNextToken());
  }

  if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
    SortColumn(_("Modified"), SORT_KEEP);
  else
    SortColumnReset();

  if (synced)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      GetFileName(), 
      wxExFrame::STAT_SYNC | wxExFrame::STAT_FULLPATH);
#endif
  }
}

void wxExListViewFile::DoFileSave(bool save_as)
{
  for (int i = 0; i < GetItemCount(); i++)
  {
    Write(ItemToText(i) + wxTextFile::GetEOL());
  }
}

wxExListViewFile::ListType wxExListViewFile::GetTypeTool(const wxExTool& tool)
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
    default: wxFAIL; return LIST_PROJECT;
  }
}

const wxString wxExListViewFile::GetTypeDescription(ListType type)
{
  wxString value;

  switch (type)
  {
  case LIST_COUNT: value = _("File Count"); break;
  case LIST_FIND: value = _("Find Results"); break;
  case LIST_HISTORY: value = _("History"); break;
  case LIST_KEYWORD: value = _("Keywords"); break;
  case LIST_PROCESS: value = _("Process Output"); break;
  case LIST_PROJECT: value = _("Project"); break;
  case LIST_REPLACE: value = _("Replace Results"); break;
  case LIST_REVISION: value = _("Revisions"); break;
  case LIST_SQL: value = _("SQL Queries"); break;
  case LIST_VERSION: value = _("Version List"); break;
  default: wxFAIL;
  }

  return value;
}

void wxExListViewFile::Initialize(const wxExLexer* lexer)
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

#ifndef __WXMSW__
  // Under Linux this should be done before adding any columns, under MSW it does not matter!
  SetSingleStyle(wxLC_REPORT);
#else
  // Set initial style depending on type.
  // Might be improved.
  SetSingleStyle((m_Type == LIST_PROJECT || m_Type == LIST_HISTORY ?
    wxConfigBase::Get()->ReadLong("List/Style", wxLC_REPORT) :
    wxLC_REPORT));
#endif

  const int col_line_width = 750;

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
      std::set<wxString>::const_iterator it = lexer->GetKeywords().begin();
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

#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new ListViewDropTarget(this));
#endif
}

void wxExListViewFile::ItemsUpdate()
{
  for (int i = 0; i < GetItemCount(); i++)
  {
    wxExListItemWithFileName item(this, i);
    item.Update();
  }
}

bool wxExListViewFile::ItemFromText(const wxString& text)
{
  if (text.empty())
  {
    return false;
  }

  wxStringTokenizer tkz(text, GetFieldSeparator());
  if (tkz.HasMoreTokens())
  {
    const wxString value = tkz.GetNextToken();
    wxFileName fn(value);

    if (fn.FileExists())
    {
      wxExListItemWithFileName item(this, value);
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
          item.SetColumnText(col, value);
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

      wxExListItemWithFileName dir(this, value, findfiles);
      dir.Insert();
    }
  }
  else
  {
    wxExListItemWithFileName item(this, text);
    item.Insert();
  }

  m_ContentsChanged = true;

  return true;
}

const wxString wxExListViewFile::ItemToText(int item_number) const
{
  wxExListItemWithFileName item(
    const_cast< wxExListViewFile * >(this), item_number);

  wxString text = (item.GetFileName().GetStat().IsOk() ? 
    item.GetFileName().GetFullPath(): 
    item.GetFileName().GetFullName());

  if (wxFileName::DirExists(item.GetFileName().GetFullPath()))
  {
    text += GetFieldSeparator() + GetItemText(item_number, _("Type"));
  }

  if (GetType() != LIST_PROJECT)
  {
    text += GetFieldSeparator() + wxExListView::ItemToText(item_number);
  }

  return text;
}

void wxExListViewFile::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() > ID_EDIT_SVN_LOWEST && event.GetId() < ID_EDIT_SVN_HIGHEST)
  {
    wxExSVN svn(
      event.GetId(), 
      wxExListItemWithFileName(this, GetNextSelected(-1)).GetFileName().GetFullPath());
    svn.ExecuteAndShowOutput(this);
    return;
  }

  switch (event.GetId())
  {
  // These are added to disable changing this listview if it is read-only etc.
  case wxID_CLEAR:
  case wxID_CUT:
  case wxID_DELETE:
  case wxID_PASTE:
    if (m_Type == LIST_HISTORY)
    {
      // Do nothing.
    }
    else if (GetFileName().GetStat().IsOk())
    {
      if (!GetFileName().GetStat().IsReadOnly())
      {
        event.Skip();
        m_ContentsChanged = true;
      }
    }
    else
    {
      event.Skip();
      m_ContentsChanged = false;
    }
  break;

  case wxID_ADD: AddItems(); break;

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

#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void wxExListViewFile::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  if (
    !IsShown() ||
     GetItemCount() == 0 ||
     !wxConfigBase::Get()->ReadBool("AllowSync", true))
  {
    return;
  }

  if (m_ItemNumber < GetItemCount())
  {
    wxExListItemWithFileName item(this, m_ItemNumber);

    if ( item.GetFileName().FileExists() &&
        (item.GetFileName().GetStat().GetModificationTime() != 
         GetItemText(m_ItemNumber, _("Modified")) ||
         item.GetFileName().GetStat().IsReadOnly() != item.IsReadOnly())
        )
    {
      item.Update();
#if wxUSE_STATUSBAR
      wxExFrame::StatusText(
        item.GetFileName(), 
        wxExFrame::STAT_SYNC | wxExFrame::STAT_FULLPATH);
#endif
      m_ItemUpdated = true;
    }

    m_ItemNumber++;
  }
  else
  {
    m_ItemNumber = 0;

    if (m_ItemUpdated)
    {
      if (wxConfigBase::Get()->ReadBool("List/SortSync", true) && 
          m_Type == LIST_PROJECT)
      {
        SortColumn(_("Modified"), SORT_KEEP);
      }

      m_ItemUpdated = false;
    }
  }

  CheckFileSync();
}

void wxExListViewFile::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED)
  {
#if wxUSE_STATUSBAR
    if (GetSelectedItemCount() == 1)
    {
      const wxExListItemWithFileName item(this, event.GetIndex());

      if (item.GetFileName().GetStat().IsOk())
      {
        wxExFrame::StatusText(
          item.GetFileName(), 
          wxExFrame::STAT_FULLPATH);
      }
    }
#endif

    event.Skip();
  }
  else
  {
    wxFAIL;
  }
}

void wxExListViewFile::OnMouse(wxMouseEvent& event)
{
  if (event.LeftDown())
  {
    event.Skip();
    int flags = wxLIST_HITTEST_ONITEM;
    const long index = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

    // If no item has been selected, then show filename mod time in the statusbar.
    if (index < 0)
    {
      if (GetFileName().FileExists())
      {
#if wxUSE_STATUSBAR
        wxExFrame::StatusText(GetFileName());
#endif
      }
    }
  }
  else
  {
    wxFAIL;
  }

#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void wxExListViewFile::SetStyle(int id)
{
  long view = 0;
  switch (id)
  {
  case wxID_VIEW_DETAILS: view = wxLC_REPORT; break;
  case wxID_VIEW_LIST: view = wxLC_LIST; break;
  case wxID_VIEW_SMALLICONS: view = wxLC_SMALL_ICON; break;
  default: wxFAIL;
  }

  SetSingleStyle(view);
  wxConfigBase::Get()->Write("List/Style", view);
}

#if wxUSE_DRAG_AND_DROP
bool ListViewDropTarget::OnDropFiles(
  wxCoord, 
  wxCoord, 
  const wxArrayString& filenames)
{
  for (size_t n = 0; n < filenames.GetCount(); n++)
  {
    m_Owner->ItemFromText(filenames[n]);
  }

  if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
  {
    m_Owner->SortColumn(_("Modified"), SORT_KEEP);
  }

  return true;
}
#endif

#ifdef __WXMSW__
#ifdef wxExUSE_RBS
RBSFile::RBSFile(wxExListViewFile* listview)
  : wxExFile()
  , m_Owner(listview)
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
  int i = -1;
  while ((i = m_Owner->GetNextSelected(i)) != -1)
  {
    wxExListItemWithFileName li(m_Owner, i);
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

  wxExLog::Get()->Log("RBS " + _("File") + ": " + script + " " + _("generated"));
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
  Write("' Script generated by: " + wxTheApp->GetAppName() + ": " + wxDateTime::Now().Format() + "\n" +
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

BEGIN_EVENT_TABLE(wxExListViewWithFrame, wxExListViewFile)
  EVT_LIST_ITEM_ACTIVATED(wxID_ANY, wxExListViewWithFrame::OnList)
  EVT_MENU_RANGE(ID_LIST_LOWEST, ID_LIST_HIGHEST, wxExListViewWithFrame::OnCommand)
  EVT_MENU_RANGE(ID_TOOL_LOWEST, ID_TOOL_HIGHEST, wxExListViewWithFrame::OnCommand)
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
  : wxExListViewFile(parent, type, id, menu_flags, lexer, pos, size, style, validator, name)
  , m_Frame(frame)
{
  if (GetType() == LIST_HISTORY)
  {
    m_Frame->UseFileHistoryList(this);
  }
}

wxExListViewWithFrame::wxExListViewWithFrame(wxWindow* parent,
  wxExFrameWithHistory* frame,
  const wxString& file,
  wxWindowID id,
  long menu_flags,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString &name)
  : wxExListViewFile(parent, file, id, menu_flags, pos, size, style, validator, name)
  , m_Frame(frame)
{
  if (GetFileName().GetStat().IsOk())
  {
    m_Frame->SetRecentProject(file);
  }
}

void wxExListViewWithFrame::BuildPopupMenu(wxExMenu& menu)
{
  bool exists = true;
  bool is_folder = false;
  bool read_only = false;
  bool is_make = false;

  if (GetSelectedItemCount() == 1)
  {
    const wxExListItemWithFileName item(this, GetFirstSelected());

    exists = item.GetFileName().GetStat().IsOk();
    is_folder = wxDirExists(item.GetFileName().GetFullPath());
    read_only = item.GetFileName().GetStat().IsReadOnly();
    is_make = item.GetFileName().GetLexer().GetScintillaLexer() == "makefile";
  }

  if (GetSelectedItemCount() >= 1 && exists)
  {
    menu.Append(ID_LIST_OPEN_ITEM, _("&Open"), wxART_FILE_OPEN);
    menu.AppendSeparator();
  }

  wxExListViewFile::BuildPopupMenu(menu);

  if (GetSelectedItemCount() > 1 && !wxConfigBase::Get()->Read(_("Comparator")).empty())
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

    if ( GetType() != LIST_PROJECT &&
        !wxExSVN::Get()->Use() &&
         exists && !is_folder)
    {
      wxExListViewFile* list = m_Frame->Activate(LIST_PROJECT);

      if (list != NULL && list->GetSelectedItemCount() == 1)
      {
        wxExListItemWithFileName thislist(this, GetFirstSelected());
        const wxString current_file = thislist.GetFileName().GetFullPath();

        wxExListItemWithFileName otherlist(list, list->GetFirstSelected());
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
      if (!wxExSVN::Get()->Use())
      {
        menu.AppendSeparator();

        if (!wxConfigBase::Get()->Read(_("Comparator")).empty())
        {
          menu.Append(ID_LIST_COMPARELAST, _("&Compare Recent Version"));
        }

        menu.Append(ID_LIST_VERSIONLIST, _("&Version List"));
      }
      else if (GetSelectedItemCount() == 1)
      {
        const wxExListItemWithFileName item(this, GetFirstSelected());
        menu.AppendSeparator();

        if (wxExSVN::Get()->DirExists(item.GetFileName()))
        {
          menu.AppendSVN();
        }
      }
    }

    // Finding in the LIST_FIND and REPLACE would result in recursive calls, do not add them.
    if ( exists &&
         GetType() != LIST_FIND && GetType() != LIST_REPLACE &&
        (GetMenuFlags() & LIST_MENU_REPORT_FIND))
    {
      menu.AppendSeparator();
      menu.Append(ID_TOOL_REPORT_FIND, wxExEllipsed(GetFindInCaption(ID_TOOL_REPORT_FIND)));

      if (!read_only)
      {
        menu.Append(ID_TOOL_REPORT_REPLACE, wxExEllipsed(GetFindInCaption(ID_TOOL_REPORT_REPLACE)));
      }
    }
  }

  if (GetSelectedItemCount() > 0 && exists && (GetMenuFlags() & LIST_MENU_TOOL))
  {
    menu.AppendSeparator();
    menu.AppendTools();
  }
}

void wxExListViewWithFrame::DeleteDoubles()
{
  wxDateTime mtime((time_t)0);
  wxString name;
  const int itemcount = GetItemCount();

  for (int i = itemcount - 1; i >= 0; i--)
  {
    wxExListItemWithFileName item(this, i);

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
    // Next gives compile error, but is not needed.
    // m_ContentsChanged = true;
  }
}

const wxString wxExListViewWithFrame::GetFindInCaption(int id)
{
  const wxString prefix =
    (id == ID_TOOL_REPORT_REPLACE ?
       _("Replace In"):
       _("Find In")) + " ";

  if (GetSelectedItemCount() == 1)
  {
    // The File Name is better than using 0, as it can be another column as well.
    return prefix + GetItemText(GetFirstSelected(), _("File Name"));
  }
  else if (GetSelectedItemCount() > 1)
  {
    return prefix + _("Selection");
  }
  else if (!GetFileName().GetName().empty())
  {
    return prefix + GetFileName().GetName();
  }
  else
  {
    return prefix + GetName();
  }
}

void wxExListViewWithFrame::DoFileLoad(bool synced)
{
  wxExListViewFile::DoFileLoad(synced);
  m_Frame->SetRecentProject(GetFileName().GetFullPath());
}

void wxExListViewWithFrame::ItemActivated(int item_number)
{
  wxASSERT(item_number >= 0);
 
  // Cannot be const because of SetColumnText later on.
  wxExListItemWithFileName item(this, item_number);

  if (wxFileName::DirExists(item.GetFileName().GetFullPath()))
  {
    wxTextEntryDialog dlg(this,
      _("Input") + ":",
      _("Folder Type"),
      GetItemText(item_number, _("Type")));

    if (dlg.ShowModal() == wxID_OK)
    {
      item.SetColumnText(_("Type"), dlg.GetValue());
    }
  }
  else if (item.GetFileName().FileExists())
  {
    const wxString line_number_str = GetItemText(item_number, _("Line No"), false);
    const int line_number = (!line_number_str.empty() ? atoi(line_number_str.c_str()): 0);
    const wxString match =
      (GetType() == LIST_REPLACE ?
         GetItemText(item_number, _("Replaced")):
         GetItemText(item_number, _("Match"), false));

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
    return;
  }

  switch (event.GetId())
  {
  case ID_LIST_OPEN_ITEM:
  {
    int i = -1;
    while ((i = GetNextSelected(i)) != -1)
    {
      ItemActivated(i);
    }
  }
  break;

  case ID_LIST_COMPARE:
  case ID_LIST_COMPARELAST:
  case ID_LIST_VERSIONLIST:
  {
    bool first = true;
    wxString file1,file2;
    int i = -1;
    bool found = false;

    wxExListViewWithFrame* list = NULL;

    if (event.GetId() == ID_LIST_VERSIONLIST)
    {
      list = m_Frame->Activate(LIST_VERSION);
      wxASSERT(list != NULL);
    }

    while ((i = GetNextSelected(i)) != -1)
    {
      wxExListItemWithFileName li(this, i);
      const wxFileName* filename = &li.GetFileName();
      if (wxFileName::DirExists(filename->GetFullPath())) continue;
      switch (event.GetId())
      {
        case ID_LIST_COMPARE:
        {
          if (GetSelectedItemCount() == 1)
          {
            list = m_Frame->Activate(LIST_PROJECT);
            if (list == NULL) return;
            int main_selected = list->GetFirstSelected();
            wxExCompareFile(wxExListItemWithFileName(list, main_selected).GetFileName(), *filename);
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
          if (wxExFindOtherFileName(*filename, NULL, &lastfile))
          {
            wxExCompareFile(*filename, lastfile);
          }
        }
        break;
        case ID_LIST_VERSIONLIST:
          if (wxExFindOtherFileName(*filename, list, NULL))
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
  {
    const wxExListItemWithFileName item(this, GetNextSelected(-1));
    wxExMake(m_Frame, item.GetFileName());
  }
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
    wxExSTC* stc = m_Frame->GetSTC();

    if (stc != NULL)
    {
      stc->GetSearchText();
    }

    std::vector<wxExConfigItem> v;

    v.push_back(wxExConfigItem(
      wxExFindReplaceData::Get()->GetTextFindWhat(), 
      CONFIG_COMBOBOX, 
      wxEmptyString, 
      true));

    if (tool.GetId() == ID_TOOL_REPORT_REPLACE) 
    {
      v.push_back(wxExConfigItem(
        wxExFindReplaceData::Get()->GetTextReplaceWith(), 
        CONFIG_COMBOBOX));
    }

    v.push_back(wxExConfigItem());
    v.push_back(wxExConfigItem(wxExFindReplaceData::Get()->GetInfo()));

    if (wxExConfigDialog(this,
      v,
      GetFindInCaption(tool.GetId())).ShowModal() == wxID_CANCEL)
    {
      return;
    }

    wxExLog::Get()->Log(wxExFindReplaceData::Get()->GetText(
      tool.GetId() == ID_TOOL_REPORT_REPLACE));
  }

  if (!wxExTextFileWithListView::SetupTool(tool, m_Frame))
  {
    return;
  }

  int i = -1;

  wxExStatistics<long> stats;

  while ((i = GetNextSelected(i)) != -1)
  {
    stats += wxExListItemWithFileName(this, i).Run(tool).GetElements();
  }

  tool.Log(&stats, GetFileName().GetName());

  if (tool.IsCount())
  {
    m_Frame->OpenFile(
      tool.GetLogfileName(), 0 , wxEmptyString, wxExSTC::STC_OPEN_FROM_OTHER);
  }
}
