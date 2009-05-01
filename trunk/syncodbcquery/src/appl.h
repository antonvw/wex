/******************************************************************************\
* File:          appl.h
* Purpose:       Declaration of classes for syncquery
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#define USE_OTL 1
#include <wx/extension/report/report.h>

enum
{
  ID_FIRST,
  ID_DATABASE_CLOSE,
  ID_DATABASE_OPEN,
  ID_RECENTFILE_MENU,
  ID_VIEW_QUERY,
  ID_VIEW_RESULTS,
  ID_VIEW_STATISTICS,
  ID_LAST,
};

class MyApp: public wxExApp
{
public:
  MyApp() {}
  virtual bool OnInit();
private:
  DECLARE_NO_COPY_CLASS(MyApp)
};

class MyFrame: public wxExFrameWithHistory
{
public:
  MyFrame(const wxString& title);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual void  ConfigDialogApplied(wxWindowID dialogid);
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  void RunQuery(const wxString& query, bool empty_results = false);
  void RunQueries(const wxString& text);
  void UpdateStatistics(const wxStopWatch& sw, long rpc);
  wxExSTCShell* m_Shell;
  wxExSTCWithFrame* m_Query;
  wxExGrid* m_Results;
  wxExStatistics <long> m_Statistics;
  otl_connect m_db;
  bool m_Running;
  bool m_Stopped;

  DECLARE_NO_COPY_CLASS(MyFrame)
  DECLARE_EVENT_TABLE()
};
