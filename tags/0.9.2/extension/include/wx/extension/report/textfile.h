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
class wxExListView;

/// Offers a wxExTextFile with reporting to a listview.
class WXDLLIMPEXP_BASE wxExTextFileWithListView : public wxExTextFile
{
public:
  /// Constructor.
  wxExTextFileWithListView(
    const wxExFileName& filename,
    const wxExTool& tool);

  /// Sets up the tool.
  static bool SetupTool(const wxExTool& tool, wxExFrameWithHistory* frame);
private:
  // Implement interface from wxExTextFile.
  virtual void Report(size_t line);
  virtual void ReportStatistics();

  static wxExListView* m_Report;
  static wxExFrameWithHistory* m_Frame;

#if wxExUSE_EMBEDDED_SQL
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
