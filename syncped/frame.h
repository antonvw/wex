////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _FRAME_H
#define _FRAME_H

#include <wx/extension/notebook.h>
#include <wx/extension/process.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "support.h"

class Frame : public DecoratedFrame
{
public:
  Frame(bool open_recent);
  
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnNotebook(wxAuiNotebookEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  void AddAsciiTable(wxExSTC* stc);
  void AddPaneHistory();
  void AddPaneProjects();
  wxExListViewWithFrame* AddPage(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  bool DialogProjectOpen();
  wxExSTC* ExecExCommand(int command);
  virtual wxExListViewFile* GetProject();
  void NewFile(bool as_project = false);
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
  virtual void StatusBarDoubleClicked(const wxString& pane);
  virtual void StatusBarDoubleClickedRight(const wxString& pane);
  virtual void SyncCloseAll(wxWindowID id);

  bool m_IsClosing;
  int m_NewFileNo;
  int m_NewProjectNo;
  int m_SplitId;

  wxExGenericDirCtrl* m_DirCtrl;
  wxExNotebook* m_Editors;
  wxExListViewWithFrame* m_History;
  wxExNotebook* m_Lists;
  wxExProcess* m_Process;
  wxExNotebook* m_Projects;

  const long m_PaneFlag;
  const wxString m_ProjectWildcard;

  DECLARE_EVENT_TABLE()
};
#endif
