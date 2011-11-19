////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of sample classes for wxExtension report
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/treectrl.h>
#include <wx/extension/app.h>
#include <wx/extension/notebook.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/stc.h>

/// Derive your application from wxExApp.
class wxExRepSampleApp: public wxExApp
{
public:
  wxExRepSampleApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(wxExRepSampleApp)
};

/// Use wxExFrameWithHistory.
class wxExRepSampleFrame: public wxExFrameWithHistory
{
public:
  /// Constructor.
  wxExRepSampleFrame();
protected:
  // Interface from wxExFrameWithHistory.
  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType type, 
    const wxExLexer* lexer = NULL);
  virtual wxExListView* GetListView();
  virtual wxExSTC* GetSTC();
  virtual bool OpenFile(
    const wxExFileName& file,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  void OnCommand(wxCommandEvent& event);
private:
  wxExNotebook* m_NotebookWithLists; ///< all listviews
  wxExSTCWithFrame* m_STC;           ///< an stc
  DECLARE_EVENT_TABLE()
};
