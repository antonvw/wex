////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/notebook.h>
#include <wx/extension/process.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "support.h"

class App;

class Notebook : public wxExNotebook
{
public:
  Notebook(wxWindow* parent,
    wxExManagedFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxAUI_NB_DEFAULT_STYLE);
};

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
    long flags = 0) override;
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL) override;
  virtual bool ExecExCommand(const std::string& command, wxExSTC* & stc) override;
  virtual wxExListViewFile* GetProject() override;
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY) override;
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
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  void AddPaneHistory();
  void AddPaneProjects();
  bool DialogProjectOpen();
  void NewFile(const wxString& name);
  void NewProject();

  const long m_PaneFlag;
  const wxString m_ProjectWildcard;

  bool m_IsClosing;
  int m_NewProjectNo;
  int m_SplitId;

  App* m_App;
  Notebook* m_Editors;

  wxCheckBox* m_CheckBoxDirCtrl;
  wxCheckBox* m_CheckBoxHistory;
  wxExGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History;
  wxExNotebook* m_Lists;
  wxExProcess* m_Process;
  wxExNotebook* m_Projects;
  wxExSTC* m_asciiTable;

  DECLARE_EVENT_TABLE()
};
