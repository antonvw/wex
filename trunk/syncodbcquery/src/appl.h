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
#include <wx/filetool/filetool.h>

enum
{
  ID_FIRST,
  ID_DATABASE_CLOSE,
  ID_DATABASE_OPEN,
  ID_QUERY_RUN,
  ID_RECENTFILE_MENU,
  ID_OPTIONS,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TOOLBAR,
  ID_VIEW_QUERY,
  ID_VIEW_RESULTS,
  ID_VIEW_STATISTICS,
  ID_LAST,
};

class MyApp: public exApp
{
public:
  MyApp() {}
  virtual bool OnInit();
private:
  DECLARE_NO_COPY_CLASS(MyApp)
};

class MyFrame: public ftFrame
{
public:
  MyFrame(const wxString& title);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual void  ConfigDialogApplied(wxWindowID dialogid);
  virtual bool OpenFile(const wxString& file,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  void RunQuery(const wxString& query, bool empty_results = false);
  void RunQueries(const wxString& text);
  void UpdateStatistics(const wxStopWatch& sw, long rpc);
  exSTCShell* m_Shell;
  ftSTC* m_Query;
  exGrid* m_Results;
  exStatistics <long> m_Statistics;
  otl_connect m_db;
  bool m_Stopped;

  DECLARE_NO_COPY_CLASS(MyFrame)
  DECLARE_EVENT_TABLE()
};
