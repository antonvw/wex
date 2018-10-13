////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/app.h>
#include <wx/extension/otl.h>
#include <wx/extension/statistics.h>
#include <wx/extension/report/frame.h>

enum
{
  ID_FIRST,
  ID_DATABASE_CLOSE,
  ID_DATABASE_OPEN,
  ID_RECENTFILE_MENU,
  ID_VIEW_QUERY,
  ID_VIEW_RESULTS,
  ID_VIEW_STATISTICS,
  ID_LAST 
};

namespace wex
{
  class grid;
  class shell;
  class stc;
};

class app: public wex::app
{
public:
  app() {}
  virtual bool OnInit() override;
};

class frame: public wex::history_frame
{
public:
  frame();
private:
  virtual void OnCommandItemDialog(
    wxWindowID dialogid, 
    const wxCommandEvent& event) override;
  virtual wex::stc* OpenFile(
    const wex::path& filename, 
    const wex::stc_data& data = wex::stc_data()) override;
  virtual void StatusBarClicked(const std::string& pane) override;

  void RunQuery(const std::string& query, bool empty_results = false);

  wex::grid* m_Results;
  wex::otl m_otl;
  wex::shell* m_Shell;
  wex::statistics <int> m_Statistics;
  wex::stc* m_Query;
  
  bool m_Running = false;
  bool m_Stopped = false;
};
