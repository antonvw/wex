/******************************************************************************\
* File:          frame.h
* Purpose:       Declaration of class 'MDIFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: frame.h 49 2008-11-12 19:08:42Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _MDIFRAME_H
#define _MDIFRAME_H

#include <wx/generic/dirctrlg.h>
#include <wx/extension/extension.h>
#include <wx/extension/notebook.h>
#include "support.h"

/// Adds notebooks to Frame.
class MDIFrame : public Frame
{
public:
  /// Constructor.
  MDIFrame(bool open_recent);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  // Interface from ftFrame.
  virtual ftListView* Activate(int type, const exLexer* lexer = NULL);
  virtual ftListView* GetCurrentProject();
  virtual ftSTC* GetCurrentSTC();
  // Interface from exFrame.
  virtual void ConfigDialogApplied(wxWindowID dialogid);
  virtual exSTC* GetSTC() {return GetCurrentSTC();}
  virtual exListView* GetListView();
  virtual bool OpenFile(const wxString& file, 
    int line_number = 0, 
    const wxString& match = wxEmptyString, 
    long flags = 0);
  virtual void SyncCloseAll(wxWindowID id);

  ftListView* AddPage(int type, const exLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  void NewFile(bool as_project = false);

  int m_NewFileNo;

  wxGenericDirCtrl* m_DirCtrl;
  ftListView* m_History;
  exNotebook* m_NotebookWithEditors;
  exNotebook* m_NotebookWithLists;
  exNotebook* m_NotebookWithProjects;
  exSTC* m_AsciiTable;

  DECLARE_EVENT_TABLE()
};
#endif
