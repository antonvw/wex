////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of class Frame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _FRAME_H
#define _FRAME_H

#include <wx/extension/notebook.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "support.h"

class Frame : public DecoratedFrame
{
public:
  Frame(bool open_recent);
 ~Frame();
  
  void Log(
    wxLogLevel level,
    const wxString& msg,
    const wxLogRecordInfo& info);
  
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
    wxExListViewFileName::ListType type, 
    const wxExLexer* lexer = NULL);
  void AddAsciiTable(wxExSTC* stc);
  void AddHeader(wxExSTC* stc);
  wxExListViewWithFrame* AddPage(
    wxExListViewFileName::ListType type, 
    const wxExLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  bool DialogProjectOpen();
  virtual wxExListViewFile* GetProject();
  void NewFile(bool as_project = false);
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxExVCSEntry& vcs,
    long flags = 0);
  void SequenceDialog(wxExSTC* stc);
  virtual void StatusBarDoubleClicked(const wxString& pane);
  virtual void StatusBarDoubleClickedRight(const wxString& pane);
  virtual void SyncCloseAll(wxWindowID id);

  int m_NewFileNo;
  int m_NewProjectNo;

  wxExGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History;
  wxExNotebook* m_Editors;
  wxExNotebook* m_Lists;
  wxExNotebook* m_Projects;

  const wxString m_ProjectWildcard;
  wxString m_LogFile;
  wxString m_LogText;
  wxLog* m_OldLog;

  DECLARE_EVENT_TABLE()
};
#endif
