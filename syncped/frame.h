////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0,
    const wxString& command = wxEmptyString) override;
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListView* Activate(
    wxExListView::wxExListType type, 
    const wxExLexer* lexer = nullptr) override;
  virtual bool ExecExCommand(const std::string& command, wxExSTC* & stc) override;
  virtual wxExListViewFile* GetProject() override;
  virtual void OnCommandItemDialog(
    wxWindowID dialogid,
    const wxCommandEvent& event) override;
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxExVCSEntry& vcs,
    long flags = 0) override;
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxString& text,
    long flags = 0) override;
  virtual void PrintEx(wxExEx* ex, const wxString& text) override;
  virtual void StatusBarClicked(const wxString& pane) override;
  virtual void StatusBarClickedRight(const wxString& pane) override;
  virtual void SyncAll() override;
  virtual void SyncCloseAll(wxWindowID id) override;
  
  void AddAsciiTable();
  wxExListViewWithFrame* AddPage(
    wxExListView::wxExListType type, 
    const wxExLexer* lexer = nullptr);
  void AddPaneHistory();
  void AddPaneProcess();
  void AddPaneProjects();
  bool DialogProjectOpen();
  void NewFile(const wxString& name);
  void NewProject();

  const long m_PaneFlag;
  const wxString m_ProjectWildcard;

  bool m_IsClosing;
  int m_NewProjectNo, m_SplitId;

  App* m_App;

  wxCheckBox *m_CheckBoxDirCtrl, *m_CheckBoxHistory;
  wxExGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History = nullptr;
  wxExNotebook *m_Editors, *m_Lists, *m_Projects = nullptr;
  wxExProcess* m_Process;
  wxExSTC* m_asciiTable = nullptr;

  DECLARE_EVENT_TABLE()
};
