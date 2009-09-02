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
#include <wx/extension/stc.h>
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
  // Interface from wxExFrameWithHistory.
  virtual wxExListViewFile* Activate(
    wxExListViewFile::ListType type, 
    const wxExLexer* lexer = NULL);
  virtual wxExListViewFile* GetCurrentProject();
  virtual wxExSTCWithFrame* GetCurrentSTC();
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxString& unique,
    const wxString& contents);
  // Interface from wxExFrame.
  virtual void ConfigDialogApplied(wxWindowID dialogid);
  virtual wxExSTC* GetSTC() {return GetCurrentSTC();}
  virtual wxExListView* GetListView();
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  virtual void SyncCloseAll(wxWindowID id);

  wxExListViewFile* AddPage(
    wxExListViewFile::ListType type, 
    const wxExLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  void NewFile(bool as_project = false);

  int m_NewFileNo;

  wxGenericDirCtrl* m_DirCtrl;
  wxExListViewFile* m_History;
  wxExNotebook* m_NotebookWithEditors;
  wxExNotebook* m_NotebookWithLists;
  wxExNotebook* m_NotebookWithProjects;

  DECLARE_EVENT_TABLE()
};
#endif
