////////////////////////////////////////////////////////////////////////////////
// Name:      listviewfile.h
// Purpose:   Declaration of class wxExListViewFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
    const std::string& folder,
    const std::string& files,
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
  const auto& GetTextAddFiles() const {return m_TextAddFiles;};
  const auto& GetTextAddFolders() const {return m_TextAddFolders;};
  const auto& GetTextAddRecursive() const {return m_TextAddRecursive;};
  const auto& GetTextAddWhat() const {return m_TextAddWhat;};
  const auto& GetTextInFolder() const {return m_TextInFolder;};

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
  
  const std::string 
    m_TextAddFiles = _("Add files"),
    m_TextAddFolders = _("Add folders"),
    m_TextAddRecursive = _("Recursive"),
    m_TextAddWhat = _("Add what"),
    m_TextInFolder = _("In folder");
  
  wxExItemDialog* m_AddItemsDialog;
};
