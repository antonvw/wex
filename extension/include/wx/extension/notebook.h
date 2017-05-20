////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.h
// Purpose:   Declaration of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <wx/aui/auibook.h>
#include <wx/config.h>
#include <wx/wupdlock.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/window-data.h>

#if wxUSE_GUI

class wxExManagedFrame;

/// Offers a notebook with page access using keys,
/// and that interfaces with wxExManagedFrame.
class WXDLLIMPEXP_BASE wxExNotebook : public wxAuiNotebook
{
public:
  /// Default constructor.
  wxExNotebook(const wxExWindowData& data = wxExWindowData().Style(wxAUI_NB_DEFAULT_STYLE));

  /// Adds the page with given key and fills the keys.
  wxWindow* AddPage(
    wxWindow* page,
    const std::string& key,
    const std::string& text = std::string(), // in that case uses key as text
    bool select = false,
    const wxBitmap& bitmap = wxNullBitmap);

  /// Changes the selection for the given page, returning the previous selection.
  /// If the key does not exist an empty string is returned.
  const std::string ChangeSelection(const std::string& key);

  /// Deletes the page with the given key.
  /// Returns true if page could be deleted.
  bool DeletePage(const std::string& key);

  /// Do something for each page in the notebook.
  /// The id should be inbetween ID_ALL_LOWEST and ID_ALL_HIGHEST.
  /// Cannot be const as it can call DeletePage.
  template <class T> 
  bool ForEach(int id) {
    wxWindowUpdateLocker locker(m_Frame != nullptr ? (wxWindow*)m_Frame: (wxWindow*)this);
    
    // The page should be an int (no), otherwise page >= 0 never fails!
    for (int page = GetPageCount() - 1; page >= 0; page--)
    {
      T* win = (T*)GetPage(page);
      switch (id)
      {
      case ID_ALL_CLOSE:
      case ID_ALL_CLOSE_OTHERS:
        if ((id == ID_ALL_CLOSE_OTHERS && GetSelection() != page) ||
             id == ID_ALL_CLOSE)
        {
          if (wxExFileDialog(
            &win->GetFile()).ShowModalIfChanged() == wxID_CANCEL) 
          {
            return false;
          }
          const std::string key = m_Windows[GetPage(page)];
          m_Windows.erase(GetPage(page));
          m_Keys.erase(key);
          wxAuiNotebook::DeletePage(page);
        }
        break;

      case ID_ALL_CONFIG_GET: win->ConfigGet(); break;
        
      case ID_ALL_SAVE:
        if (win->GetFile().GetContentsChanged())
        {
          win->GetFile().FileSave();
        }
        break;

      // STC only!!!
      case ID_ALL_STC_SET_LEXER: 
        // At this moment same as themed change,
        // as we want default colour updates as well.
        ((wxExSTC*)GetPage(page))->GetLexer().Set(((wxExSTC*)GetPage(page))->GetLexer().GetDisplayLexer());
        break;

      case ID_ALL_STC_SET_LEXER_THEME: 
        ((wxExSTC*)GetPage(page))->GetLexer().Set(((wxExSTC*)GetPage(page))->GetLexer().GetDisplayLexer());
        break;

      case ID_ALL_STC_SYNC: 
        ((wxExSTC*)GetPage(page))->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true)); 
        break;
        
      default: 
        wxFAIL; 
        break;
      }
    }
    if (m_Frame != nullptr && m_Keys.empty())
    {
      m_Frame->SyncCloseAll(GetId());
    }
    return true;};
  
  /// Returns key for the current page.
  const std::string GetCurrentPage();

  /// Returns the key specified by the given page.
  /// If the page does not exist or is nullptr an empty string is returned.
  const std::string GetKeyByPage(wxWindow* page) const {
    if (page == nullptr) return std::string();
    const auto& it = m_Windows.find(page);
    return (it != m_Windows.end() ? it->second: std::string());};
  
  /// Returns the page specified by the given key.
  /// If the key does not exist nullptr is returned.
  wxWindow* GetPageByKey(const std::string& key) const {
    const auto& it = m_Keys.find(key);
    return (it != m_Keys.end() ? it->second: nullptr);};
  
  /// Returns the page index specified by the given key.
  /// If the key does not exist wxNOT_FOUND is returned.
  int GetPageIndexByKey(const std::string& key) const {
    wxWindow* page = GetPageByKey(key);
    return (page != nullptr ? GetPageIndex(page): wxNOT_FOUND);};
  
  /// Inserts the page with given key and fills the keys.
  wxWindow* InsertPage(
    size_t page_idx,
    wxWindow* page,
    const std::string& key,
    const std::string& text = std::string(), // in that case uses key as text
    bool select = false,
    const wxBitmap& bitmap = wxNullBitmap);

  /// Rearranges all pages.
  void Rearrange(
    /// Specify where the pane should go.
    /// It should be one of the following: 
    /// - wxTOP
    /// - wxBOTTOM
    /// - wxLEFT
    /// - wxRIGHT
    int direction);

  /// Sets the pagetext for the given new key,
  /// on the page for the given key.
  /// If the key does not exist false is returned.
  bool SetPageText(
    const std::string& key,
    const std::string& new_key,
    const std::string& text,
    const wxBitmap& bitmap = wxNullBitmap);
      
  /// Selects (and returns) the page specified by the given key.
  /// If the key does not exist nullptr is returned.
  wxWindow* SetSelection(const std::string& key);
  
  /// Split performs a split operation programmatically. 
  /// If the key does not exist false is returned.
  bool Split(
    /// The page that will be split off. 
    /// This page will also become the active page after the split.
    const std::string& key, 
    /// Specify where the pane should go.
    /// It should be one of the following: 
    /// - wxTOP
    /// - wxBOTTOM
    /// - wxLEFT
    /// - wxRIGHT
    int direction);
private:
  wxExManagedFrame* m_Frame;
  // In bookctrl.h: m_pages
  std::map<std::string, wxWindow*> m_Keys;
  std::map<wxWindow*, std::string> m_Windows;
};
#endif // wxUSE_GUI
