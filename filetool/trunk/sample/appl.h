/******************************************************************************\
* File:          appl.h
* Purpose:       Declaration of sample classes for wxFileTool
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: appl.h 40 2008-11-10 18:19:59Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/treectrl.h>
#include <wx/generic/dirctrlg.h>
#include <wx/extension/notebook.h>

class ftSampleFrame;

/// Derive your application from exApp.
class ftSampleApp: public exApp
{
public:
  ftSampleApp() {}
private:
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(ftSampleApp)
};

/// Use ftFrame.
class ftSampleFrame: public ftFrame
{
public:
  /// Constructor.
  ftSampleFrame(const wxString& title);
protected:
  // Interface from ftFrame.
  virtual ftListView* Activate(int type, const exLexer* lexer = NULL);
  virtual ftSTC* GetCurrentSTC() {return m_STC;};
  virtual bool OpenFile(const wxString& file, 
    int line_number = 0, 
    const wxString& match = wxEmptyString, 
    long flags = 0);
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
private:
  exNotebook* m_NotebookWithLists; ///< all listviews
  ftSTC* m_STC;                    ///< an stc
  wxGenericDirCtrl* m_DirCtrl;
  DECLARE_EVENT_TABLE()
};
