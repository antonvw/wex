/******************************************************************************\
* File:          frame.h
* Purpose:       Declaration of class 'MDIFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _MDIFRAME_H
#define _MDIFRAME_H

#include <wx/generic/dirctrlg.h>
#include <wx/extension/notebook.h>
#include "support.h"

class MDIFrame : public Frame
{
public:
  MDIFrame(bool open_recent);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListViewStandard* Activate(
    wxExListViewStandard::ListType type, 
    const wxExLexer* lexer = NULL);
  bool DialogProjectOpen();
  virtual wxExListView* GetListView();
  virtual wxExListViewFile* GetProject();
  virtual wxExSTCFile* GetSTC();
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxString& unique,
    const wxString& contents,
    long flags = 0);
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  virtual void SyncCloseAll(wxWindowID id);

  wxExListViewWithFrame* AddPage(
    wxExListViewStandard::ListType type, 
    const wxExLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  void NewFile(bool as_project = false);

  int m_NewFileNo;
  int m_NewProjectNo;

  wxGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History;
  wxExNotebook* m_NotebookWithEditors;
  wxExNotebook* m_NotebookWithLists;
  wxExNotebook* m_NotebookWithProjects;

  const wxString m_ProjectWildcard;

  DECLARE_EVENT_TABLE()
};
#endif
