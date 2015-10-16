////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
  virtual bool OnInit();
private:
  DECLARE_NO_COPY_CLASS(App)
};

class Frame: public wxExFrameWithHistory
{
public:
  Frame();
private:
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    const wxCommandEvent& event) override;
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0);
  void RunQuery(const wxString& query, bool empty_results = false);
  void RunQueries(const wxString& text);
  void UpdateStatistics(long time, long rpc);

  wxExGrid* m_Results;
  wxExSTC* m_Query;
  wxExShell* m_Shell;
  
  wxExStatistics <int> m_Statistics;
  wxExOTL m_otl;
  
  bool m_Running;
  bool m_Stopped;

  DECLARE_NO_COPY_CLASS(Frame)
};
