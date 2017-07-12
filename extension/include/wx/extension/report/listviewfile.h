////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.h
// Purpose:   Declaration of class wxExListViewFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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
  wxExListViewFile(
    const std::string& file,
    const wxExListViewData& data = wxExListViewData());

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
  virtual bool ItemFromText(const std::string& text) override;

  /// Resets the member.
  virtual void ResetContentsChanged() override {m_ContentsChanged = false;};
protected:
  virtual void BuildPopupMenu(wxExMenu& menu) override;
  virtual bool DoFileLoad(bool synced = false) override;
  virtual void DoFileNew() override;
  virtual void DoFileSave(bool save_as = false) override;
  void OnIdle(wxIdleEvent& event);
private:
  bool m_ContentsChanged = false;
  const wxString m_TextAddFiles = _("Add files");
  const wxString m_TextAddFolders = _("Add folders");
  const wxString m_TextAddRecursive = _("Recursive");
  const wxString m_TextAddWhat = _("Add what");
  const wxString m_TextInFolder = _("In folder");
  
  wxExItemDialog* m_AddItemsDialog;
};
