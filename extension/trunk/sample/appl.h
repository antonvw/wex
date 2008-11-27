/******************************************************************************\
* File:          appl.h
* Purpose:       Declaration of sample classes for wxExtension
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/extension.h>
#include <wx/extension/dir.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/notebook.h>
#include <wx/extension/stc.h>
#include <wx/extension/shell.h>

/// Derive your application from exApp.
class exSampleApp: public exApp
{
public:
  /// Constructor.
  exSampleApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(exSampleApp)
};

/// Use exDir.
class exSampleDir: public exDir
{
public:
  /// Constructor.
  exSampleDir(const wxString& fullpath, const wxString& findfiles, exGrid* grid);
private:
  /// Override the OnFile.
  virtual void OnFile(const wxString& file);
  exGrid* m_Grid; ///< put it in a grid
};

/// Use exManagedFrame.
class exSampleFrame: public exManagedFrame
{
public:
  /// Constructor.
  exSampleFrame(const wxString& title);
protected:
  virtual void ConfigDialogApplied(wxWindowID id);
  /// Do something.
  void OnCommand(wxCommandEvent& event);
private:
  exGrid* m_Grid;         ///< a grid
  exListView* m_ListView; ///< a listview
  exNotebook* m_Notebook; ///< a notebook
  exSTCShell* m_STCShell; ///< an stc shell
  exToolBar* m_ToolBar;   ///< a notebook
  exSTC* m_STC;           ///< an stc

  long m_FlagsSTC;        ///< keep current flags
  exStatistics <long> m_Statistics; ///< keep some statistics

  DECLARE_NO_COPY_CLASS(exSampleFrame)
  DECLARE_EVENT_TABLE()
};
