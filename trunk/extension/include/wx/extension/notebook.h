/******************************************************************************\
* File:          notebook.h
* Purpose:       Declaration of class exNotebook
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXNOTEBOOK_H
#define _EXNOTEBOOK_H

#include <wx/aui/auibook.h>
#include <wx/extension/base.h> //for exManagedFrame

#if wxUSE_GUI

/// Offers a notebook with page mapping and interfaces with exManagedFrame.
class exNotebook : public wxAuiNotebook
{
public:
  /// Constructor.
  exNotebook(wxWindow* parent,
    exManagedFrame* frame, // NULL is allowed
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxAUI_NB_DEFAULT_STYLE);

  /// Adds the page with given key and fills the map.
  /// If the key already exists, NULL is returned,
  /// and no new page is added.
  wxWindow* AddPage(
    wxWindow* page,
    const wxString& key,
    const wxString& text = wxEmptyString, // in that case uses key as text
    bool select = false,
    const wxBitmap& bitmap = wxNullBitmap) {
    if (GetPageByKey(key, select) != NULL)
    {
      wxLogError(FILE_INFO("Page with key: %s already exists"), key.c_str());
      return NULL;
    }
    const wxString use_text = (text.empty() ? key: text);
    if (!wxAuiNotebook::AddPage(page, use_text, select, bitmap))
    {
      wxLogError(FILE_INFO("Could not add page with text: %s"), use_text.c_str());
      return NULL;
    }
    m_MapPages[key] = page;
    return page;}

  /// Deletes the page with the given key.
  bool DeletePage(const wxString& key) {
    return wxAuiNotebook::DeletePage(GetPageIndex(m_MapPages[key]));};

  /// Do something for each page in the notebook.
  /// The pages should all be castable to exSTC pages.
  /// The id should be inbetween ID_ALL_LOWEST and ID_ALL_HIGHEST.
  bool ForEach(int id);

  /// Returns the key specified by the given page.
  /// If the page does not exist an empty string is returned.
  const wxString GetKeyByPage(wxWindow* page) const;

  /// Returns the page specified by the given key.
  /// If the key does not exist NULL is returned.
  wxWindow* GetPageByKey(const wxString& key, bool select = false) {
    std::map<wxString,wxWindow*>::const_iterator it = m_MapPages.find(key);
    if (it == m_MapPages.end()) return NULL;
    if (select)
      SetSelection(GetPageIndex(it->second));
    return it->second;}

  /// Gets the pages map.
  const std::map<wxString, wxWindow*> & GetMapPages() const {return m_MapPages;};

  /// Sets the pagetext for the given key.
  /// If the key does not exist false is returned.
  bool SetPageText(
    const wxString& key,
    const wxString& text);

  /// Sets the pagetext for the given new key,
  /// on the page for the given key.
  /// If the key does not exist false is returned.
  bool SetPageText(
    const wxString& key,
    const wxString& new_key,
    const wxString& text);
protected:
  void OnNotebook(wxAuiNotebookEvent& event);
private:
  bool ErasePage(size_t n); // remove from the map

  exManagedFrame* m_Frame;
  // In bookctrl.h: m_pages
  std::map<wxString, wxWindow*> m_MapPages;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
