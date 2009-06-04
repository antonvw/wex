/******************************************************************************\
* File:          textfile.h
* Purpose:       Declaration of class 'wxExTextFileWithListView'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_TEXTFILE_H
#define _EX_REPORT_TEXTFILE_H

#include <wx/extension/textfile.h>
#include <wx/extension/otl.h>

class wxExFrameWithHistory;
class wxExListViewFile;

/// Offers a wxExTextFile with reporting to a listview.
class wxExTextFileWithListView : public wxExTextFile
{
public:
  /// Constructor.
  wxExTextFileWithListView(
    const wxExFileName& filename,
    const wxExTool& tool);

  /// Sets up the tool.
  static bool SetupTool(const wxExTool& tool);

#if USE_EMBEDDED_SQL
  // Called by wxExFrameWithHistory::OnClose.
  // Not for doxygen.
  static void CleanUp();
#endif
private:
  // Implement interface from wxExTextFile.
  virtual void Report();
  virtual void ReportStatistics();

  static wxExListViewFile* m_Report;
  static wxExFrameWithHistory* m_Frame;

#if USE_EMBEDDED_SQL
  virtual bool ParseComments();
  bool ParseSQL();
  bool SetSQLQuery();

  bool m_SQLResultsParsing;

  static wxExOTL m_otl;

  wxString m_SQLQuery;
  wxString m_SQLQueryRunTime;
#endif
};
#endif
