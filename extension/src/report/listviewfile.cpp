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
#include <wx/xml/xml.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>

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
  , m_ItemUpdated(false)
  , m_ItemNumber(0)
{
  FileLoad(file);
  
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

wxExListViewFile::~wxExListViewFile()
{
  m_AddItemsDialog->Destroy();
}

void wxExListViewFile::AddItems()
{
  int flags = 0;

  if (wxConfigBase::Get()->ReadBool(m_TextAddFiles, true)) 
  {
    flags |= wxDIR_FILES;
  }

  if (wxConfigBase::Get()->ReadBool(m_TextAddRecursive, true)) 
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

  const auto added = GetItemCount() - old_count;

  if (added > 0)
  {
    m_ContentsChanged = true;

    if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
    {
      SortColumn(_("Modified"), SORT_KEEP);
    }
  }

  const wxString text = 
    _("Added") + wxString::Format(" %d ", added) + _("file(s)");

  wxLogStatus(text);
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
  const long style = wxExMenu::MENU_DEFAULT |
    (GetFileName().GetStat().IsReadOnly() ? wxExMenu::MENU_IS_READ_ONLY: 0);

  menu.SetStyle(style);

  wxExListViewWithFrame::BuildPopupMenu(menu);
    
  if ( GetSelectedItemCount() == 0 &&
      !GetFileName().GetStat().IsReadOnly())
  {
    menu.AppendSeparator();
    menu.Append(wxID_ADD);
  }
}

void wxExListViewFile::DoFileLoad(bool synced)
{
  EditClearAll();

  wxXmlDocument doc;

  if (!doc.Load(GetFileName().GetFullPath()))
  {
    return;
  }

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    const wxString value = child->GetNodeContent();

    if (child->GetName() == "file")
    {
      wxExListItem(this, value).Insert();
    }
    else if (child->GetName() == "folder")
    {
      wxExListItem(this, value, child->GetAttribute("extensions")).Insert();
    }
    
    child = child->GetNext();
  }

  if (synced)
  {
    wxExLogStatus(GetFileName(), STAT_SYNC | STAT_FULLPATH);
  }

  GetFrame()->SetRecentProject(GetFileName().GetFullPath());
}

void wxExListViewFile::DoFileNew()
{
  EditClearAll();
}

void wxExListViewFile::DoFileSave(bool save_as)
{
  wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "files");
  wxXmlNode* comment = new wxXmlNode(
    wxXML_COMMENT_NODE,
    wxEmptyString,
    wxTheApp->GetAppDisplayName() + " project " + GetFileName().GetFullName() + 
      " "  + wxDateTime::Now().Format());

  root->AddChild(comment);

  for (auto i = 0; i < GetItemCount(); i++)
  {
    const wxExFileName fn = wxExListItem(this, i).GetFileName();

    wxXmlNode* element = new wxXmlNode(
      wxXML_ELEMENT_NODE,
      (fn.FileExists() ? "file": "folder"));

    if (!fn.FileExists() && fn.DirExists())
    {
      element->AddAttribute("extensions", GetItemText(i, _("Type")));
    }
    
    wxXmlNode* text = new wxXmlNode(
      wxXML_TEXT_NODE, 
      wxEmptyString, 
      fn.GetFullPath());
      
    element->AddChild(text);
    root->AddChild(element);
  }
  
  wxXmlDocument doc;
  doc.SetRoot(root);
  doc.Save(GetFileName().GetFullPath());
}

bool wxExListViewFile::ItemFromText(const wxString& text)
{
  m_ContentsChanged = true;
  return wxExListViewStandard::ItemFromText(text);
}

void wxExListViewFile::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  // These are added to disable changing this listview if it is read-only.
  case wxID_CLEAR:
  case wxID_CUT:
  case wxID_DELETE:
  case wxID_PASTE:
    if (!GetFileName().GetStat().IsReadOnly())
    {
      event.Skip();
      m_ContentsChanged = true;
    }
    break;

  case wxID_ADD:   
    // Force at least one of the checkboxes to be checked.
    m_AddItemsDialog->ForceCheckBoxChecked(_("Add"));
    m_AddItemsDialog->Show();
    break;

  default: 
    wxFAIL;
    break;
  }

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this);
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
    wxExListItem item(this, m_ItemNumber);

    if ( item.GetFileName().FileExists() &&
        (item.GetFileName().GetStat().GetModificationTime() != 
         GetItemText(m_ItemNumber, _("Modified")) ||
         item.GetFileName().GetStat().IsReadOnly() != item.IsReadOnly())
        )
    {
      item.Update();
      wxExLogStatus(item.GetFileName(), STAT_SYNC | STAT_FULLPATH);
      m_ItemUpdated = true;
    }

    m_ItemNumber++;
  }
  else
  {
    m_ItemNumber = 0;

    if (m_ItemUpdated)
    {
      if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
      {
        SortColumn(_("Modified"), SORT_KEEP);
      }

      m_ItemUpdated = false;
    }
  }

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

    // If no item has been selected, then show 
    // filename mod time in the statusbar.
    int flags = wxLIST_HITTEST_ONITEM;
    const auto index = HitTest(wxPoint(event.GetX(), event.GetY()), flags);

    if (index < 0)
    {
      wxExLogStatus(GetFileName());
    }
  }
  else
  {
    wxFAIL;
  }
}
