////////////////////////////////////////////////////////////////////////////////
// Name:      filehistory.cpp
// Purpose:   Implementation of wxExFileHistory class methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <experimental/filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/filehistory.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/path.h>
#include <wx/extension/util.h>

class wxExFileHistoryImp : public wxFileHistory
{
public:
  wxExFileHistoryImp(
    size_t maxFiles = 9, 
    wxWindowID idBase = wxID_FILE1)
    : wxFileHistory(maxFiles, idBase) {;};

  virtual void AddFileToHistory(const wxString& file) override;
  virtual wxString GetHistoryFile(size_t index = 0) const override;
};

wxExFileHistory::wxExFileHistory(
  size_t maxFiles, wxWindowID idBase, const std::string& key)
  : m_History(new wxExFileHistoryImp(maxFiles, idBase))
  , m_Key(key)
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  if (!key.empty())
  {
    for (int i = GetMaxFiles() - 1 ; i >=0 ; i--)
    {
      m_History->AddFileToHistory(
        wxConfigBase::Get()->Read(wxString::Format("%s/%d", key.c_str(), i)));
    }
  }
}

wxExFileHistory::~wxExFileHistory()
{
  delete m_History;
}
  
void wxExFileHistory::AddFileToHistory(const wxExPath& file)
{
  if (file.FileExists())
  {
    m_History->AddFileToHistory(file.Path().string());
  }
}

void wxExFileHistory::Clear()
{
  if (GetCount() > 0)
  {
    for (int i = GetCount() - 1; i >= 0; i--)
    {
      m_History->RemoveFileFromHistory(i);
    }
  }
}

wxWindowID wxExFileHistory::GetBaseId() const
{
  return m_History->GetBaseId();
}
  
size_t wxExFileHistory::GetCount() const
{
  return m_History->GetCount();
}
  
int wxExFileHistory::GetMaxFiles() const
{
  return m_History->GetMaxFiles();
}

wxExPath wxExFileHistory::GetHistoryFile(size_t index) const
{
  return wxExPath(m_History->GetHistoryFile(index).ToStdString());
}
    
std::vector<wxExPath> wxExFileHistory::GetHistoryFiles(size_t count) const
{
  std::vector<wxExPath> v;
  
  for (size_t i = 0; i < count && i < GetCount(); i++)
  {
    v.emplace_back(GetHistoryFile(i));
  }  
  
  return v;
}
  
void wxExFileHistory::PopupMenu(wxWindow* win,
  int clear_id, const wxPoint& pos) const
{
  wxMenu* menu = new wxMenu();

  for (size_t i = 0; i < GetCount(); i++)
  {
    if (const wxExPath file(GetHistoryFile(i)); file.FileExists())
    {
      wxMenuItem* item = new wxMenuItem(
        menu, 
        GetBaseId() + i, 
        file.GetFullName());

      item->SetBitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
        wxExGetIconID(file)));
    
      menu->Append(item);
    }
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    menu->AppendSeparator();
    
    if (clear_id != -1)
    {
      menu->Append(clear_id, wxGetStockLabel(wxID_CLEAR));
    }
      
    win->PopupMenu(menu, pos);
  }
    
  delete menu;
}

void wxExFileHistory::Save()
{
  if (m_Key.empty())
  {
    m_History->Save(*wxConfigBase::Get());
  }
  else
  {
    if (GetCount() > 0)
    {
      wxConfigBase::Get()->DeleteGroup(m_Key);
      
      for (size_t i = 0; i < GetCount(); i++)
      {
        wxConfigBase::Get()->Write(
          wxString::Format("%s/%lu", m_Key, i),
          m_History->GetHistoryFile(i));
      }
    }
  }
}
  
void wxExFileHistory::UseMenu(wxWindowID id, wxMenu* menu)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);
  m_History->UseMenu(submenu);

  if (m_Key.empty())
  {
    // We can load file history now.
    m_History->Load(*wxConfigBase::Get());
  }
  else
  {
    m_History->AddFilesToMenu();
  }
}

// Implementation

void wxExFileHistoryImp::AddFileToHistory(const wxString& file)
{
  if (!file.empty() && GetMaxFiles() > 0)
  {
    wxFileHistory::AddFileToHistory(file);
  }
}

wxString wxExFileHistoryImp::GetHistoryFile(size_t index) const
{
  if (GetCount() > 0 && (int)index < GetMaxFiles())
  {
    bool error = false;
    wxString file;

    try
    {
      file = wxFileHistory::GetHistoryFile(index);

      if (!std::experimental::filesystem::is_regular_file(file.ToStdString()))
      {
        error = true;
      }
      else
      {
        return wxFileHistory::GetHistoryFile(index);
      }
    }
    catch (const std::exception& )
    {
      error = true;
    }

    if (error)
    {
      const_cast< wxExFileHistoryImp * >( this )->RemoveFileFromHistory(index);
      wxLogStatus(_("Removed not existing file: %s from history"), file.c_str());
    }
  }
  
  return wxEmptyString;
}
