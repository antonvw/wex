////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncodbcquery
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
private:
  DECLARE_NO_COPY_CLASS(App)
};

class Frame: public wxExFrameWithHistory
{
public:
  Frame();
private:
  virtual void OnCommandItemDialog(wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename, 
    int line_number = 0, 
    const std::sting& match = std::string(), 
    int col_number = 0, long flags = 0, 
    const std::sting& command = std::string()) override;

  void RunQuery(const wxString& query, bool empty_results = false);

  wxExGrid* m_Results;
  wxExSTC* m_Query;
  wxExShell* m_Shell;
  
  wxExStatistics <int> m_Statistics;
  wxExOTL m_otl;
  
  bool m_Running = false;
  bool m_Stopped = false;

  DECLARE_NO_COPY_CLASS(Frame)
};
