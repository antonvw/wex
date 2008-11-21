/******************************************************************************\
* File:          textfile.h
* Purpose:       Declaration of class 'ftTextFile'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2007, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTTEXTFILE_H
#define _FTTEXTFILE_H

#include <wx/extension/otl.h>

/// Offers an exTextFile with reporting to a listview.
class ftTextFile : public exTextFile
{
public:
  /// Constructor.
  ftTextFile(const exFileName& filename);

  /// Sets up the tool.
  static bool SetupTool(const exTool& tool);

#if USE_EMBEDDED_SQL
  // Called by ftFrame::OnClose.
  // Not for doxygen.
  static void CleanUp();
#endif
private:
  // Implement interface from exTextFile.
  virtual bool Cancelled();
  virtual void Report();
  virtual void ReportStatistics();

  static ftListView* m_Report;
  static ftFrame* m_Frame;

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
