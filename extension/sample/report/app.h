////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of sample classes for wxExtension report
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
    const wxExLexer* lexer = nullptr) override;
  virtual bool AllowClose(wxWindowID id, wxWindow* page) override;
  virtual wxExListView* GetListView() override;
  virtual wxExSTC* GetSTC() override;
  virtual wxExSTC* OpenFile(
    const wxExPath& file,
    const wxExSTCData& stc_data = wxExSTCData()) override;
private:
  wxExNotebook* m_NotebookWithLists; ///< all listviews
  wxExSTC* m_STC;
};
