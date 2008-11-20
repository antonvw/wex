/******************************************************************************\
* File:          notebook.h
* Purpose:       Declaration of class exNotebook
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: notebook.h 53 2008-11-13 18:38:57Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXNOTEBOOK_H
#define _EXNOTEBOOK_H

#include <wx/aui/auibook.h>

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
    const wxBitmap& bitmap = wxNullBitmap);

  /// Deletes the page with the given key.
  bool DeletePage(const wxString& key);

  /// Do something for each page in the notebook.
  /// The pages should all be castable to exSTC pages.
  /// The id should be inbetween ID_ALL_LOWEST and ID_ALL_HIGHEST.
  bool ForEach(int id);

  /// Returns the page specified by the given key.
  /// If the key does not exist NULL is returned.
  wxWindow* GetPageByKey(const wxString& key, bool select = false);

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
