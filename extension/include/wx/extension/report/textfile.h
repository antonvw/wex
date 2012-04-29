////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.h
// Purpose:   Declaration of class 'wxExTextFileWithListView'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  static bool SetupTool(
    /// tool to use
    const wxExTool& tool, 
    /// frame
    wxExFrameWithHistory* frame,
    /// listview to which is reported, if NULL,
    /// calls Activate on frame to find report
    wxExListView* report = NULL);
private:
  /// Inserts a line at current line (or at end if at end),
  /// make that line current and sets modified.
  void InsertLine(const wxString& line);

  // Implement interface from wxExTextFile.
  virtual void Report(size_t line);
  virtual void ReportKeyword();

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
