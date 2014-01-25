////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.h
// Purpose:   Declaration of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXNOTEBOOK_H
#define _EXNOTEBOOK_H

#include <map>
#include <wx/aui/auibook.h>

#if wxUSE_GUI

class wxExManagedFrame;

/// Offers a notebook with page access using keys,
/// and that interfaces with wxExManagedFrame.
class WXDLLIMPEXP_BASE wxExNotebook : public wxAuiNotebook
{
public:
  /// Constructor.
  wxExNotebook(wxWindow* parent,
    wxExManagedFrame* frame, // NULL is allowed
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxAUI_NB_DEFAULT_STYLE);

  /// Adds the page with given key and fills the keys.
  wxWindow* AddPage(
    wxWindow* page,
    const wxString& key,
    const wxString& text = wxEmptyString, // in that case uses key as text
    bool select = false,
    const wxBitmap& bitmap = wxNullBitmap);

  /// Deletes the page with the given key.
  /// Returns true if page could be deleted.
  bool DeletePage(const wxString& key);

  /// Do something for each page in the notebook.
  /// The pages should all be castable to wxExSTC pages.
  /// The id should be inbetween ID_ALL_LOWEST and ID_ALL_HIGHEST.
  /// Cannot be const as it can call DeletePage.
  bool ForEach(int id);

  /// Returns the key specified by the given page.
  /// If the page does not exist an empty string is returned.
  const wxString GetKeyByPage(wxWindow* page) const {
    const auto it = m_Windows.find(page);
    return (it != m_Windows.end() ? it->second: wxString(wxEmptyString));};
  
  /// Returns the page specified by the given key.
  /// If the key does not exist NULL is returned.
  wxWindow* GetPageByKey(const wxString& key) const {
    const auto it = m_Keys.find(key);
    return (it != m_Keys.end() ? it->second: NULL);};
  
  /// Returns the page index specified by the given key.
  /// If the key does not exist wxNOT_FOUND is returned.
  int GetPageIndexByKey(const wxString& key) const {
    wxWindow* page = GetPageByKey(key);
    return (page != NULL ? GetPageIndex(page): wxNOT_FOUND);};
  
  /// Inserts the page with given key and fills the keys.
  wxWindow* InsertPage(
    size_t page_idx,
    wxWindow* page,
    const wxString& key,
    const wxString& text = wxEmptyString, // in that case uses key as text
    bool select = false,
    const wxBitmap& bitmap = wxNullBitmap);

  /// Sets the pagetext for the given new key,
  /// on the page for the given key.
  /// If the key does not exist false is returned.
  bool SetPageText(
    const wxString& key,
    const wxString& new_key,
    const wxString& text,
    const wxBitmap& bitmap = wxNullBitmap);
      
  /// Selects (and returns) the page specified by the given key.
  /// If the key does not exist NULL is returned.
  wxWindow* SetSelection(const wxString& key);
protected:
  void OnNotebook(wxAuiNotebookEvent& event);
private:
  wxExManagedFrame* m_Frame;
  // In bookctrl.h: m_pages
  std::map<wxString, wxWindow*> m_Keys;
  std::map<wxWindow*, wxString> m_Windows;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
