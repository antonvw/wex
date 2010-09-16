////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.cpp
// Purpose:   Implementation of class 'wxExListViewFile'
// Author:    Anton van Wezenbeek
// Created:   2010-01-29
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>

BEGIN_EVENT_TABLE(wxExListViewFile, wxExListViewWithFrame)
  EVT_IDLE(wxExListViewFile::OnIdle)
  EVT_MENU(wxID_ADD, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_CUT, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_CLEAR, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_DELETE, wxExListViewFile::OnCommand)
  EVT_MENU(wxID_PASTE, wxExListViewFile::OnCommand)
  EVT_LEFT_DOWN(wxExListViewFile::OnMouse)
END_EVENT_TABLE()

wxExListViewFile::wxExListViewFile(wxWindow* parent,
  wxExFrameWithHistory* frame,
  const wxString& file,
  wxWindowID id,
  long menu_flags,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  const wxString& name)
  : wxExListViewWithFrame(
      parent, 
      frame, 
      LIST_FILE, 
      id, 
      menu_flags, 
      NULL, 
      pos, 
      size, 
      style, 
      validator, 
      name)
  , wxExFile(false) // do not open files in FileLoad and Save
  , m_AddItemsDialog(NULL)
  , m_ContentsChanged(false)
  , m_TextAddFiles(_("Add files"))
  , m_TextAddFolders(_("Add folders"))
  , m_TextAddRecursive(_("Recursive"))
  , m_TextAddWhat(_("Add what"))
  , m_TextInFolder(_("In folder"))
{
  FileLoad(file);
}

wxExListViewFile::~wxExListViewFile()
{
  if (m_AddItemsDialog != NULL)
  {
    m_AddItemsDialog->Destroy();
  }
}

void wxExListViewFile::AddItems()
{
  int flags = 0;

  if (wxConfigBase::Get()->ReadBool(m_TextAddFiles, true)) 
  {
    flags |= wxDIR_FILES;
  }

  if (wxConfigBase::Get()->ReadBool(m_TextAddRecursive, false)) 
  {
    flags |= wxDIR_DIRS;
  }

  wxExDirWithListView dir(
    this,
    wxExConfigFirstOf(m_TextInFolder),
    wxExConfigFirstOf(m_TextAddWhat),
    flags);

  const auto old_count = GetItemCount();

  dir.FindFiles();

  const auto new_count = GetItemCount();

  if (new_count - old_count > 0)
  {
    m_ContentsChanged = true;

    if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
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

void wxExListViewFile::AddItemsDialog()
{
  if (m_AddItemsDialog == NULL)
  {
    std::vector<wxExConfigItem> v;

    v.push_back(wxExConfigItem(
      m_TextAddWhat, 
      CONFIG_COMBOBOX, 
      wxEmptyString, 
      true));

    v.push_back(wxExConfigItem(
      m_TextInFolder, 
      CONFIG_COMBOBOXDIR, 
      wxEmptyString, 
      true,
      1000));

    std::set<wxString> set;
    
    set.insert(m_TextAddFiles);
    set.insert(m_TextAddFolders);
    set.insert(m_TextAddRecursive);

    v.push_back(wxExConfigItem(set));

    m_AddItemsDialog = new wxExConfigDialog(this,
      v,
      _("Add Items"),
      0,
      1,
      wxOK | wxCANCEL,
      wxID_ADD);
  }

  // Force at least one of the checkboxes to be checked.
  m_AddItemsDialog->ForceCheckBoxChecked(_("Add"));
  m_AddItemsDialog->Show();
}

void wxExListViewFile::AfterSorting()
{
  // Only if we are not sort syncing set contents changed.
  if (!wxConfigBase::Get()->ReadBool("List/SortSync", true))
  {
    m_ContentsChanged = true;
  }
}

void wxExListViewFile::BuildPopupMenu(wxExMenu& menu)
{
  // This contains the CAN_PASTE flag.
  long style = wxExMenu::MENU_DEFAULT;

  if (GetFileName().FileExists() && GetFileName().GetStat().IsReadOnly())
  {
    style |= wxExMenu::MENU_IS_READ_ONLY;
  }

  menu.SetStyle(style);

  wxExListViewWithFrame::BuildPopupMenu(menu);
    
  if ((GetSelectedItemCount() == 0) &&
      (!GetFileName().IsOk() || !GetFileName().FileExists() ||
      (GetFileName().FileExists() && !GetFileName().GetStat().IsReadOnly())))
  {
    menu.AppendSeparator();
    menu.Append(wxID_ADD);
  }
}

void wxExListViewFile::DoFileLoad(bool synced)
{
  EditClearAll();

  const wxCharBuffer& buffer = Read();

  ItemFromText(buffer.data());

  m_ContentsChanged = false;

  if (synced)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      GetFileName(), 
      wxExFrame::STAT_SYNC | wxExFrame::STAT_FULLPATH);
#endif
  }

  GetFrame()->SetRecentProject(GetFileName().GetFullPath());
}

void wxExListViewFile::DoFileNew()
{
  EditClearAll();
}

void wxExListViewFile::DoFileSave(bool save_as)
{
  for (auto i = 0; i < GetItemCount(); i++)
  {
    Write(ItemToText(i) + wxTextFile::GetEOL());
  }
}

bool wxExListViewFile::ItemFromText(const wxString& text)
{
  if (wxExListViewStandard::ItemFromText(text))
  {
    m_ContentsChanged = true;
    return true;
  }
  else
  {
    return false;
  }
}

void wxExListViewFile::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  // These are added to disable changing this listview if it is read-only etc.
  case wxID_CLEAR:
  case wxID_CUT:
  case wxID_DELETE:
  case wxID_PASTE:
    if (GetFileName().GetStat().IsOk())
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
	}
  break;

  case wxID_ADD: AddItemsDialog(); break;

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
    IsShown() &&
    GetItemCount() > 0)
  {
    CheckSync();
  }
}

void wxExListViewFile::OnMouse(wxMouseEvent& event)
{
  if (event.LeftDown())
  {
    event.Skip();

#if wxUSE_STATUSBAR
    // If no item has been selected, then show 
    // filename mod time in the statusbar.
    int flags = wxLIST_HITTEST_ONITEM;
    const auto index = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

    if (index < 0)
    {
      if (GetFileName().FileExists())
      {
        wxExFrame::StatusText(GetFileName());
      }
    }
#endif
  }
  else
  {
    wxFAIL;
  }
}
