////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/notebook.h>
#include <wx/extension/process.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "support.h"

class App;

class Frame : public DecoratedFrame
{
public:
  explicit Frame(App* app);
  
  bool IsClosing() const {return m_IsClosing;};
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListView* Activate(wxExListView::wxExListType type, const wxExLexer* lexer = nullptr) override;
  virtual bool ExecExCommand(const std::string& command, wxExSTC* & stc) override;
  virtual wxExListViewFile* GetProject() override;
  virtual bool IsOpen(const wxExFileName& filename) override;
  virtual void OnCommandItemDialog(wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const std::string& match = std::string(),
    int col_number = 0,
    wxExSTCWindowFlags flags = STC_WIN_DEFAULT,
    const std::string& command = std::string()) override;
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename, 
    const wxExVCSEntry& vcs, 
    wxExSTCWindowFlags flags = STC_WIN_DEFAULT) override;
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename, 
    const std::string& text, 
    wxExSTCWindowFlags flags = STC_WIN_DEFAULT) override;
  virtual void PrintEx(wxExEx* ex, const std::string& text) override;
  virtual wxExProcess* Process(const std::string& command) override;
  virtual wxExSTC* RestorePage(const std::string& key) override;
  virtual bool SaveCurrentPage(const std::string& key) override;
  virtual void StatusBarClicked(const std::string& pane) override;
  virtual void StatusBarClickedRight(const std::string& pane) override;
  virtual void SyncAll() override;
  virtual void SyncCloseAll(wxWindowID id) override;
  
  void AddPaneHistory();
  void AddPaneProcess();
  void AddPaneProjects();

  const long m_PaneFlag;
  const wxString m_ProjectWildcard;

  bool m_IsClosing = false;
  int m_NewProjectNo = 1, m_SplitId = 1;

  wxCheckBox *m_CheckBoxDirCtrl, *m_CheckBoxHistory;
  wxExGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History = nullptr;
  wxExNotebook *m_Editors, *m_Lists, *m_Projects = nullptr;
  wxExProcess* m_Process;
  wxExSTC* m_asciiTable = nullptr;
  std::string m_SavedPage;

  DECLARE_EVENT_TABLE()
};
