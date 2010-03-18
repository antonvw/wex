////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.cpp
// Purpose:   Implementation of class 'wxExListViewFile'
// Author:    Anton van Wezenbeek
// Created:   2010-01-29
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#include <wx/dnd.h> 
#include <wx/tokenzr.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>

#if wxUSE_DRAG_AND_DROP
class ListViewDropTarget : public wxFileDropTarget
{
public:
  ListViewDropTarget(wxExListViewFile* owner) {m_Owner = owner;}
private:
  virtual bool OnDropFiles(
    wxCoord x, 
    wxCoord y, 
    const wxArrayString& filenames);
  wxExListViewFile* m_Owner;
};
#endif

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
  , m_AddItemsDialog(NULL)
  , m_ContentsChanged(false)
  , m_TextAddFiles(_("Add files"))
  , m_TextAddFolders(_("Add folders"))
  , m_TextAddRecursive(_("Recursive"))
  , m_TextAddWhat(_("Add what"))
  , m_TextInFolder(_("In folder"))
{
#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new ListViewDropTarget(this));
#endif

  wxExFile::FileLoad(file);

  if (GetFileName().GetStat().IsOk())
  {
    m_Frame->SetRecentProject(file);
  }
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

  const long old_count = GetItemCount();

  dir.FindFiles();

  const long new_count = GetItemCount();

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
      1000)); // TODO: fix

    v.push_back(wxExConfigItem());
    std::set<wxString> set;
    set.insert(m_TextAddFiles);
    set.insert(m_TextAddFolders);
    set.insert(m_TextAddRecursive);
    v.push_back(wxExConfigItem(set));

    m_AddItemsDialog = new wxExConfigDialog(this,
      v,
      _("Add Items"),
      0,
      2,
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

  bool exists = true;
  bool is_folder = false;

  if (GetSelectedItemCount() >= 1)
  {
    wxExListViewWithFrame::BuildPopupMenu(menu);
  }
  else
  {
    if (!GetFileName().IsOk() ||
        !GetFileName().FileExists() ||
        (GetFileName().FileExists() && !GetFileName().GetStat().IsReadOnly()))
    {
      menu.AppendSeparator();
      menu.Append(wxID_ADD);
    }

    wxExListViewWithFrame::BuildPopupMenu(menu);
  }
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

  m_ContentsChanged = false;

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

  m_Frame->SetRecentProject(GetFileName().GetFullPath());
}

void wxExListViewFile::DoFileSave(bool save_as)
{
  for (long i = 0; i < GetItemCount(); i++)
  {
    Write(ItemToText(i) + wxTextFile::GetEOL());
  }
}

void wxExListViewFile::FileNew(const wxExFileName& filename)
{
  wxExFile::FileNew(filename);
  EditClearAll();
}

const wxString wxExListViewFile::GetListInfo() const
{
  return GetFileName().GetName();
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
    GetItemCount() > 0 &&
    wxConfigBase::Get()->ReadBool("AllowSync", true))
  {
    CheckFileSync();
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
    const long index = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

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

#if wxUSE_DRAG_AND_DROP
bool ListViewDropTarget::OnDropFiles(
  wxCoord, 
  wxCoord, 
  const wxArrayString& filenames)
{
  for (
    wxArrayString::const_iterator it = filenames.begin();
    it != filenames.end();
    it++)
  {
    m_Owner->ItemFromText(*it);
  }

  if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
  {
    m_Owner->SortColumn(_("Modified"), SORT_KEEP);
  }

  return true;
}
#endif
