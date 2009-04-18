/******************************************************************************\
* File:          appl.h
* Purpose:       Declaration of sample classes for wxFileTool
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/treectrl.h>
#include <wx/generic/dirctrlg.h>
#include <wx/extension/notebook.h>

/// Derive your application from exApp.
class ftSampleApp: public exApp
{
public:
  ftSampleApp() {}
private:
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(ftSampleApp)
};

/// Use exFrameWithHistory.
class ftSampleFrame: public exFrameWithHistory
{
public:
  /// Constructor.
  ftSampleFrame(const wxString& title);
protected:
  // Interface from exFrameWithHistory.
  virtual exListViewFile* Activate(int type, const exLexer* lexer = NULL);
  virtual exSTCWithFrame* GetCurrentSTC() {return m_STC;};
  virtual bool OpenFile(
    const exFileName& file, 
    int line_number = 0, 
    const wxString& match = wxEmptyString, 
    long flags = 0);
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
private:
  exNotebook* m_NotebookWithLists; ///< all listviews
  exSTCWithFrame* m_STC;           ///< an stc
  wxGenericDirCtrl* m_DirCtrl;
  DECLARE_EVENT_TABLE()
};
