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
  Frame(App* app);
  
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  void AddAsciiTable();
  void AddPaneHistory();
  void AddPaneProjects();
  wxExListViewWithFrame* AddPage(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  bool DialogProjectOpen();
  wxExSTC* ExecExCommand(int command);
  virtual wxExListViewFile* GetProject();
  void NewFile(const wxString& name);
  void NewProject();
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxExVCSEntry& vcs,
    long flags = 0);
  virtual bool OpenFile(
    const wxString& filename,
    const wxString& text,
    long flags = 0);
  virtual void StatusBarClicked(const wxString& pane);
  virtual void StatusBarClickedRight(const wxString& pane);
  virtual void SyncAll();
  virtual void SyncCloseAll(wxWindowID id);

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
