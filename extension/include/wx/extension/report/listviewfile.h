////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.h
// Purpose:   Declaration of class wxExListViewFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/file.h>
#include <wx/extension/report/listview.h>

class wxExItemDialog;

/// Combines wxExListViewWithFrame and wxExFile,
/// giving you a list control with file synchronization support.
class WXDLLIMPEXP_BASE wxExListViewFile : 
  public wxExListViewWithFrame, public wxExFile
{
public:
  /// Constructor for a LIST_FILE, opens the file.
  wxExListViewFile(wxWindow* parent,
    wxExFrameWithHistory* frame,
    const wxString& file,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_REPORT,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);

  /// Destructor.
  virtual ~wxExListViewFile();

  /// Adds items. The items are added using a separate thread,
  /// deault this thread runs detached, otherwise this method
  /// waits for the thread to finish.
  void AddItems(
    const wxString& folder,
    const wxString& files,
    long flags,
    bool detach = true);

  // Interface, for wxExListView overriden methods.
  /// Sets contents changed if we are not syncing.
  virtual void AfterSorting() override;
  
  /// Returns member.
  virtual bool GetContentsChanged() const override {return m_ContentsChanged;};

  /// Returns the file.
  wxExFile& GetFile() {return *this;};

  // Access to members.
  const wxString GetTextAddFiles() const {return m_TextAddFiles;};
  const wxString GetTextAddFolders() const {return m_TextAddFolders;};
  const wxString GetTextAddRecursive() const {return m_TextAddRecursive;};
  const wxString GetTextAddWhat() const {return m_TextAddWhat;};
  const wxString GetTextInFolder() const {return m_TextInFolder;};

  /// Adds item from text.
  virtual bool ItemFromText(const wxString& text) override;

  /// Resets the member.
  virtual void ResetContentsChanged() override {m_ContentsChanged = false;};
protected:
  virtual void BuildPopupMenu(wxExMenu& menu) override;
  virtual bool DoFileLoad(bool synced = false) override;
  virtual void DoFileNew() override;
  virtual void DoFileSave(bool save_as = false) override;
  void OnIdle(wxIdleEvent& event);
private:
  bool m_ContentsChanged;
  const wxString m_TextAddFiles;
  const wxString m_TextAddFolders;
  const wxString m_TextAddRecursive;
  const wxString m_TextAddWhat;
  const wxString m_TextInFolder;
  
  wxExItemDialog* m_AddItemsDialog;
};
