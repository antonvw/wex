////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define wxExUSE_OTL 1
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

class wxExGrid;
class wxExSTC;
class wxExShell;

class App: public wxExApp
{
public:
  App() {}
  virtual bool OnInit() override;
};

class Frame: public wxExFrameWithHistory
{
public:
  Frame();
private:
  virtual void OnCommandItemDialog(
    wxWindowID dialogid, 
    const wxCommandEvent& event) override;
  virtual wxExSTC* OpenFile(
    const wxExPath& filename, 
    const wxExSTCData& data = wxExSTCData()) override;
  virtual void StatusBarClicked(const std::string& pane) override;

  void RunQuery(const std::string& query, bool empty_results = false);

  wxExGrid* m_Results;
  wxExSTC* m_Query;
  wxExShell* m_Shell;
  
  wxExStatistics <int> m_Statistics;
  wxExOTL m_otl;
  
  bool m_Running = false;
  bool m_Stopped = false;
};
