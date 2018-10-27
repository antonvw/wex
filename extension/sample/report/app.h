////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex report sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/app.h>
#include <wex/notebook.h>
#include <wex/stc.h>
#include <wex/report/frame.h>

/// Derive your application from wex::app.
class report_sample_app: public wex::app
{
public:
  report_sample_app() {}
private:
  virtual bool OnInit() override;
};

/// Use wex::framewithhistory.
class report_sample_frame: public wex::history_frame
{
public:
  /// Constructor.
  report_sample_frame();
protected:
  // Interface from wex::framewithhistory.
  virtual wex::listview* Activate(
    wex::listview_data::type type, const wex::lexer* lexer = nullptr) override;
  virtual bool AllowClose(wxWindowID id, wxWindow* page) override;
  virtual wex::listview* GetListView() override;
  virtual wex::stc* GetSTC() override;
  virtual wex::stc* OpenFile(
    const wex::path& file,
    const wex::stc_data& stc_data = wex::stc_data()) override;
private:
  wex::notebook* m_NotebookWithLists; ///< all listviews
  wex::stc* m_STC;
};
