/******************************************************************************\
* File:          textfile.h
* Purpose:       Declaration of class 'exTextFileWithReport'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTTEXTFILE_H
#define _FTTEXTFILE_H

#include <wx/extension/textfile.h>
#include <wx/extension/otl.h>

/// Offers an exTextFile with reporting to a listview.
class exTextFileWithReport : public exTextFile
{
public:
  /// Constructor.
  exTextFileWithReport(const exFileName& filename);

  /// Sets up the tool.
  static bool SetupTool(const exTool& tool);

#if USE_EMBEDDED_SQL
  // Called by exFrameWithHistory::OnClose.
  // Not for doxygen.
  static void CleanUp();
#endif
private:
  // Implement interface from exTextFile.
  virtual void Report();
  virtual void ReportStatistics();

  static exListViewFile* m_Report;
  static exFrameWithHistory* m_Frame;

#if USE_EMBEDDED_SQL
  virtual bool ParseComments();
  bool ParseSQL();
  bool SetSQLQuery();

  static otl_connect m_db;

  bool m_SQLResultsParsing;

  wxString m_SQLQuery;
  wxString m_SQLQueryRunTime;
#endif
};
#endif
