////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.cpp
// Purpose:   Implementation of class wxExListViewFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <pugixml.hpp>
#include <wx/config.h>
#include <wx/extension/frame.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/listitem.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/frame.h>
#include <easylogging++.h>

wxExListViewFile::wxExListViewFile(const std::string& file, const wxExListViewData& data)
  : wxExListViewWithFrame(wxExListViewData(data).Type(LIST_FILE))
  , wxExFile(false) // do not open files in FileLoad and Save
  , m_AddItemsDialog(new wxExItemDialog({
        {m_TextAddWhat,ITEM_COMBOBOX, std::any(), wxExControlData().Required(true)},
        {m_TextInFolder,ITEM_COMBOBOX_DIR, std::any(), wxExControlData().Required(true)},
        {std::set<std::string> {
          m_TextAddFiles, m_TextAddFolders, m_TextAddRecursive}}},
      wxExWindowData().
        Title(_("Add Items").ToStdString()).
        Button(wxAPPLY | wxCANCEL).Id(wxID_ADD)))
{
  FileLoad(file);
  
  Bind(wxEVT_IDLE, &wxExListViewFile::OnIdle, this);
  
  Bind(wxEVT_LEFT_DOWN, [=](wxMouseEvent& event) {
    event.Skip();

    // If no item has been selected, then show 
    // filename mod time in the statusbar.
    int flags = wxLIST_HITTEST_ONITEM;

    if (const int index = HitTest(wxPoint(event.GetX(), event.GetY()), flags); index < 0)
    {
      wxExLogStatus(GetFileName());
    }
  });
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // Force at least one of the checkboxes to be checked.
    m_AddItemsDialog->ForceCheckBoxChecked(_("Add"));
    if (GetSelectedItemCount() > 0)
    {
      wxExItem item(m_AddItemsDialog->GetItem(m_TextInFolder));
      wxComboBox* cb = (wxComboBox* )item.GetWindow();
      cb->SetValue(wxExListItem(this, GetFirstSelected()).GetFileName().GetPath());
    }
    m_AddItemsDialog->Show();}, wxID_ADD);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!GetFileName().FileExists() || !GetFileName().IsReadOnly())
    {
      event.Skip();
      m_ContentsChanged = true;
#if wxUSE_STATUSBAR
      wxExFrame::UpdateStatusBar(this);
#endif
    }}, wxID_EDIT, wxID_REPLACE_ALL);
}

wxExListViewFile::~wxExListViewFile()
{
  m_AddItemsDialog->Destroy();
}

void wxExListViewFile::AddItems(
  const wxString& folder,
  const wxString& files,
  long flags,
  bool detached)
{
  Unbind(wxEVT_IDLE, &wxExListViewFile::OnIdle, this);
  
#ifdef __WXMSW__ 
  std::thread t([=] {
#endif
    const int old_count = GetItemCount();
    wxExDirWithListView dir(this, folder.ToStdString(), files.ToStdString(), flags);
  
    dir.FindFiles();
    
    const int added = GetItemCount() - old_count;
    
    if (added > 0)
    {
      m_ContentsChanged = true;
  
      if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
      {
        SortColumn(GetSortedColumnNo(), SORT_KEEP);
      }
    }
  
    const wxString text = 
      _("Added") + wxString::Format(" %d ", added) + _("file(s)");
  
    wxLogStatus(text);
    
    Bind(wxEVT_IDLE, &wxExListViewFile::OnIdle, this);
#ifdef __WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif
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
  const bool ok =
     !GetFileName().FileExists() || 
     (GetFileName().FileExists() && !GetFileName().IsReadOnly());
     
  const auto style = menu.GetStyle() | (!ok ? wxExMenu::MENU_IS_READ_ONLY: 0);

  menu.SetStyle(style);

  wxExListViewWithFrame::BuildPopupMenu(menu);
    
  if (ok)
  {
    menu.AppendSeparator();
    menu.Append(wxID_ADD);
  }
}

bool wxExListViewFile::DoFileLoad(bool synced)
{
  pugi::xml_document doc;
  const pugi::xml_parse_result result = doc.load_file(
    GetFileName().Path().string().c_str(),
    pugi::parse_default | pugi::parse_comments);

  if (!result)
  {
    wxExXmlError(GetFileName(), &result);
    return false;
  }

  EditClearAll();

// TODO: threading this on MSW does not work.
#ifdef FIX__WXMSW__ 
  std::thread t([=] {
#endif

  for (const auto& child: doc.document_element().children())
  {
    if (const std::string value = child.text().get(); strcmp(child.name(), "file") == 0)
    {
      wxExListItem(this, value).Insert();
    }
    else if (strcmp(child.name(), "folder") == 0)
    {
      wxExListItem(this, value, child.attribute("extensions").value()).Insert();
    }

    if (wxExInterruptable::Cancelled()) break;
  }

  if (synced)
  {
    wxExLogStatus(GetFileName(), STAT_SYNC | STAT_FULLPATH);
  }

  GetFrame()->SetRecentProject(GetFileName());
  
#ifdef FIX__WXMSW__ 
    });
  if (detached)  
    t.detach();
  else
    t.join();
#endif

  VLOG(1) << "opened: " << GetFileName().Path().string();

  return true;
}

void wxExListViewFile::DoFileNew()
{
  EditClearAll();
}

void wxExListViewFile::DoFileSave(bool save_as)
{
  pugi::xml_document doc;

  doc.load_string(std::string("\
    <files>\n\
    <!-- " + wxTheApp->GetAppDisplayName().ToStdString() + " project " + 
      GetFileName().GetFullName() + 
      " "  + wxDateTime::Now().Format().ToStdString().c_str() + "-->\n\
    </files>\n").c_str(),
       pugi::parse_default | pugi::parse_comments);
  
  pugi::xml_node root = doc.document_element();

  for (int i = 0; i < GetItemCount(); i++)
  {
    const wxExPath fn = wxExListItem(this, i).GetFileName();
    
    pugi::xml_node node = root.append_child(fn.FileExists() ? "file": "folder");
    node.text().set(fn.Path().string().c_str());

    if (fn.DirExists())
    {
      node.append_attribute("extensions") = 
        GetItemText(i, _("Type").ToStdString()).c_str();
    }
  }
  
  doc.save_file(GetFileName().Path().string().c_str());

  VLOG(1) << "saved: " << GetFileName().Path().string();
}

bool wxExListViewFile::ItemFromText(const std::string& text)
{
  bool result = false;
  
  if (wxExListView::ItemFromText(text))
  {
    m_ContentsChanged = true;
    result = true;
    
    if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
    {
      SortColumn(GetSortedColumnNo(), SORT_KEEP);
    }
    else
    {
      SortColumnReset();
    }
  }
  
  return result;
}

void wxExListViewFile::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    IsShown() &&
    GetItemCount() > 0 &&
    wxConfigBase::Get()->ReadBool("AllowSync", true))
  {
    CheckSync();
  }
}
