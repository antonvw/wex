/******************************************************************************\
* File:          textfile.cpp
* Purpose:       Implementation of class 'exTextFileWithReport'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/report/textfile.h>
#include <wx/extension/app.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listview.h>

exListViewFile* exTextFileWithReport::m_Report = NULL;
exFrameWithHistory* exTextFileWithReport::m_Frame = NULL;

void SetItemColumnStatistics(
  exListItemWithFileName& item,
  const wxString& col,
  exStatistics<long>& stat)
{
  item.SetColumnText(
    col,
    wxString::Format("%ld", stat.Get(col)));
}

#if USE_EMBEDDED_SQL
otl_connect exTextFileWithReport::m_db;

class Recordset
{
public:
  Recordset(otl_connect* db, exTextFile* file)
    : m_db(db)
    , m_TextFile(file)
    , m_Records(0){}
  int GetRecords() const {return m_Records;};
  bool ExecQuery(const wxString& query);
  static const wxString QueryRunTimeText() {return "QUERY RUN TIME: ";};
private:
  void UpdateTextFileFromQuery();
  otl_connect* m_db;
  otl_stream m_stream;
  exTextFile* m_TextFile;
  wxString m_Query;
  int m_Records;
};
#endif

exTextFileWithReport::exTextFileWithReport(const exFileName& filename)
  : exTextFile(filename, exApp::GetConfig(), exApp::GetLexers())
#if USE_EMBEDDED_SQL
  , m_SQLResultsParsing(false)
#endif
{
}

#if USE_EMBEDDED_SQL
void exTextFileWithReport::CleanUp()
{
  m_db.logoff();
}
#endif

#if USE_EMBEDDED_SQL
bool exTextFileWithReport::ParseComments()
{
  if (GetTool().GetId() == ID_TOOL_SQL || GetTool().GetId() == ID_TOOL_REPORT_SQL)
  {
    if (!m_SQLResultsParsing)
    {
      if (GetComments().substr(0, 4) == " SQL")
      {
        if (SetSQLQuery())
        {
          if (!ParseSQL())
          {
            return false;
          }
        }
      }
      else
      {
        ClearComments();
      }
    }
    else
    {
      if (GetTool().GetId() == ID_TOOL_REPORT_SQL &&
        GetComments().length() > Recordset::QueryRunTimeText().length() + 1 &&
        GetComments().substr(
          1,
          Recordset::QueryRunTimeText().length()) == Recordset::QueryRunTimeText())
      {
        const int start_of_runtime = Recordset::QueryRunTimeText().length() + 1;
        m_SQLQueryRunTime = GetComments().substr(start_of_runtime);
        Report();
      }

      if (GetComments().substr(0, 8) == " SQL END")
      {
        m_SQLResultsParsing = false;
      }

      ClearComments();
    }
  }

  return exTextFile::ParseComments();
}
#endif

#if USE_EMBEDDED_SQL
bool exTextFileWithReport::ParseSQL()
{
/*
 ; SQL # ..
 ;       .. #
<-- new data inserted by the recordset
 ; QUERY RUN TIME: 981201 150953
 ; USING DATABASE F\GAREX\CONFIG_MAIN.mdb
 new data
 ..
<-- end of new data
<-- old data starts here, will be deleted (the marker points to here)
 ; QUERY RUN TIME: 981201 150953
 ; USING DATABASE F\GAREX\CONFIG_MAIN.mdb
 old data
 ..
<-- end of old data
 ; SQL END (this statement triggers deletion)
*/
  if (GetTool().GetId() == ID_TOOL_SQL)
  {
    if (GetFileName().GetStat().IsReadOnly())
    {
      wxLogError("File is readonly, cannot update");
      return false;
    }

    exApp::Log(
      _("File") + ": " + GetFileName().GetFullName() + " Query: " + exSkipWhiteSpace(m_SQLQuery));

    Recordset rs(&m_db, this);
    if (!rs.ExecQuery(m_SQLQuery))
    {
      return false;
    }

    const wxString msg = wxString::Format(_("Retrieved: %d records"), rs.GetRecords());
    exApp::Log(msg);
    exFrame::StatusText(msg);
  }

  // Test for SQL end statement.
  const size_t marker = GetCurrentLine();
  ClearComments();
  m_SQLResultsParsing = true;

  while (!Eof() && m_SQLResultsParsing)
  {
    if (!ParseLine(GetNextLine()))
    {
      return false;
    }
  }

  if (m_SQLResultsParsing)
  {
    wxLogError("Missing SQL END after query: " + exSkipWhiteSpace(m_SQLQuery));
    return false;
  }

  // Remove all old lines, except the last one containing the end statement.
  if (GetTool().GetId() == ID_TOOL_SQL)
  {
    for (size_t i = GetCurrentLine() - 1; i >= marker; i--)
    {
      RemoveLine(i);
    }

    GoToLine(marker);
  }

  GetStatisticElements().Inc(_("Actions Completed"));

  return true;
}
#endif

void exTextFileWithReport::Report()
{
  exListItemWithFileName item(m_Report, GetFileName().GetFullPath());
  item.Insert();

  const int line = (GetTool().GetId() == ID_TOOL_REPORT_REVISION ?
    GetCurrentLine():
    GetCurrentLine() + 1);

  item.SetColumnText(_("Line No"), wxString::Format("%d", line));

  switch (GetTool().GetId())
  {
  case ID_TOOL_REPORT_REPLACE:
    item.SetColumnText(_("Replaced"), exApp::GetConfig()->GetFindReplaceData()->GetReplaceString());
  case ID_TOOL_REPORT_FIND:
    item.SetColumnText(_("Line"), GetRCS().GetDescription().Strip(wxString::both));
    item.SetColumnText(_("Match"), exApp::GetConfig()->GetFindReplaceData()->GetFindString());
  break;

  case ID_TOOL_REPORT_REVISION:
    item.UpdateRevisionList(GetRCS());
  break;

#if USE_EMBEDDED_SQL
  case ID_TOOL_REPORT_SQL:
    item.SetColumnText(_("Query"), m_SQLQuery);
    item.SetColumnText(_("Run Time"), m_SQLQueryRunTime);
  break;
#endif

  default: wxFAIL;
  }
}

void exTextFileWithReport::ReportStatistics()
{
  if (GetTool().GetId() == ID_TOOL_REPORT_KEYWORD)
  {
    m_Report = m_Frame->Activate(
      exListViewFile::GetTypeTool(GetTool()),
      &GetFileName().GetLexer());

    if (m_Report == NULL)
    {
      wxLogError("Report: %d is not activated", exListViewFile::GetTypeTool(GetTool()));
      return;
    }
  }

  exListItemWithFileName item(m_Report, GetFileName().GetFullPath());
  item.Insert();

  switch (GetTool().GetId())
  {
  case ID_TOOL_REPORT_COUNT:
    SetItemColumnStatistics(item, _("Lines"), GetStatisticElements());
    SetItemColumnStatistics(item, _("Lines Of Code"), GetStatisticElements());
    SetItemColumnStatistics(item, _("Empty Lines"), GetStatisticElements());
    SetItemColumnStatistics(item, _("Words Of Code"), GetStatisticElements());
    SetItemColumnStatistics(item, _("Comments"), GetStatisticElements());
    SetItemColumnStatistics(item, _("Comment Size"), GetStatisticElements());
  break;

  case ID_TOOL_REPORT_KEYWORD:
  {
    long total = 0;
    for (size_t i = 0; i < GetFileName().GetLexer().GetKeywords().size(); i++)
    {
      const wxString name = m_Report->GetColumn(i + 1).GetText();
      const long value = GetStatisticKeywords().Get(name);
      item.SetColumnText(i + 1, wxString::Format("%ld", value));
      total += value;
    }
    item.SetColumnText(
      GetFileName().GetLexer().GetKeywords().size() + 1,
      wxString::Format("%ld", total));
  }
  break;

  default: wxFAIL;
  }
}

#if USE_EMBEDDED_SQL
bool exTextFileWithReport::SetSQLQuery()
{
  const size_t pos_start_of_query = GetComments().find('#');
  const size_t pos_end_of_query = GetComments().rfind('#');

  if (pos_start_of_query == wxString::npos ||
      pos_start_of_query == pos_end_of_query)
  {
    return false;
  }

  m_SQLQuery = GetComments().substr(
    pos_start_of_query + 1,
    pos_end_of_query - pos_start_of_query - 1);

  m_SQLQuery.Trim(false);
  m_SQLQuery.Trim(true);

  return true;
}
#endif

bool exTextFileWithReport::SetupTool(const exTool& tool)
{
#if USE_EMBEDDED_SQL
  if (tool.GetId() == ID_TOOL_SQL)
  {
    if (m_db.connected)
    {
      m_db.logoff();
    }

    if (!exOTLDialog(exApp::GetConfig(), &m_db))
    {
      return false;
    }
  }
#endif

  if (tool.IsReportType())
  {
    wxWindow* window = wxTheApp->GetTopWindow();
    m_Frame = wxDynamicCast(window, exFrameWithHistory);

    if (m_Frame == NULL)
    {
      wxLogError("Cannot setup tool for exTextFileWithReport without exFrameWithHistory");
      return false;
    }

    if (tool.GetId() != ID_TOOL_REPORT_KEYWORD)
    {
      m_Report = m_Frame->Activate(exListViewFile::GetTypeTool(tool));

      if (m_Report == NULL)
      {
        wxLogError("Report: %d is not activated", exListViewFile::GetTypeTool(tool));
        return false;
      }
    }
  }

  return true;
}

#if USE_EMBEDDED_SQL
bool Recordset::ExecQuery(const wxString& query)
{
  // If the query does not contain any space,
  // assume that it specifies an existing query in the database.
  // Otherwise the statement itself is a query, pass it through as specified.
  m_Query = (!query.Contains(" ") ? "SELECT * FROM " + query: query);

  try
  {
    m_stream.set_all_column_types(otl_all_num2str | otl_all_date2str);
    m_stream.open(1024, m_Query.c_str(), *m_db);
    UpdateTextFileFromQuery();
    return true;
  }
  catch (otl_exception& p)
  {
    wxTextEntryDialog(
      NULL,
      _("Error"),
      _("ODBC Error") + ":",
      p.msg,
      wxOK | wxCENTRE | wxTE_MULTILINE | wxRESIZE_BORDER).ShowModal();
    exApp::Log(p.msg);
    return false;
  }
}

void Recordset::UpdateTextFileFromQuery()
{
  m_TextFile->GoToLine(m_TextFile->GetCurrentLine() + 1);
  m_TextFile->WriteComment(QueryRunTimeText() + wxDateTime::Now().Format(EX_TIMESTAMP_FORMAT));
  m_TextFile->WriteComment("USING ODBC DATABASE: " + exApp::GetConfig(_("Datasource")));

  // Get columns.
  int desc_len;
  m_stream.describe_select(desc_len);

   // Get all rows.
  while (!m_stream.eof())
  {
    m_Records++;
    wxString line;
    char rowstr[512];

    for (int n = 0; n < desc_len; n++)
    {
      m_stream >> rowstr;
      line += rowstr;

      if (n < desc_len - 1)
      {
        line += "\t";
      }
    }

    m_TextFile->InsertLine(line);
  }
}
#endif
