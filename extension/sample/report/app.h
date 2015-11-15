////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of sample classes for wxExtension report
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/app.h>
#include <wx/extension/notebook.h>
#include <wx/extension/stc.h>
#include <wx/extension/report/frame.h>

/// Derive your application from wxExApp.
class wxExRepSampleApp: public wxExApp
{
public:
  wxExRepSampleApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit() override;
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
  virtual wxExListView* Activate(
    wxExListView::wxExListType type, 
    const wxExLexer* lexer = NULL);
  virtual bool AllowClose(wxWindowID id, wxWindow* page) override;
  virtual wxExListView* GetListView() override;
  virtual wxExSTC* GetSTC() override;
  virtual bool OpenFile(
    const wxExFileName& file,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0) override;
private:
  wxExNotebook* m_NotebookWithLists; ///< all listviews
  wxExSTC* m_STC;
};
