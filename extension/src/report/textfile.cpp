////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.cpp
// Purpose:   Implementation of class wxExTextFileWithListView
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/frd.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listview.h>

wxExListView* wxExTextFileWithListView::m_Report = NULL;
wxExFrameWithHistory* wxExTextFileWithListView::m_Frame = NULL;

void SetItemColumnStatistics(
  wxExListItem& item,
  const wxString& col,
  const wxExStatistics<long>& stat)
{
#ifdef wxExUSE_CPP0X	
  const auto it = stat.GetItems().find(col);
#else
  std::map<wxString,long>::const_iterator it = stat.GetItems().find(col);
#endif  

  if (it != stat.GetItems().end())
  {
    item.SetItem(
      col,
      wxString::Format("%ld", it->second));
  }
}

#if wxExUSE_EMBEDDED_SQL
wxExOTL wxExTextFileWithListView::m_otl;

class Recordset
{
public:
  Recordset(otl_connect* db, wxExTextFile* file)
    : m_db(db)
    , m_TextFile(file)
    , m_Records(0){}
  size_t GetRecords() const {return m_Records;};
  bool ExecQuery(const wxString& query);
  static const wxString QueryRunTimeText() {return "QUERY RUN TIME: ";};
private:
  void UpdateTextFileFromQuery();
  /// Writes a comment to the current line.
  void WriteComment(
    const wxString& text,
    const bool fill_out = false,
    const bool fill_out_with_space = false);
  otl_connect* m_db;
  otl_stream m_stream;
  wxExTextFile* m_TextFile;
  wxString m_Query;
  size_t m_Records;
};
#endif

wxExTextFileWithListView::wxExTextFileWithListView(
  const wxExFileName& filename,
  const wxExTool& tool)
  : wxExTextFile(filename, tool)
#if wxExUSE_EMBEDDED_SQL
  , m_SQLResultsParsing(false)
#endif
{
}

#if wxExUSE_EMBEDDED_SQL
bool wxExTextFileWithListView::ParseComments()
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
        Report(GetCurrentLine());
      }

      if (GetComments().substr(0, 8) == " SQL END")
      {
        m_SQLResultsParsing = false;
      }

      ClearComments();
    }
  }

  return wxExTextFile::ParseComments();
}
#endif

#if wxExUSE_EMBEDDED_SQL
bool wxExTextFileWithListView::ParseSQL()
{
/*
 ; SQL # ..
 ;       .. #
<-- new data inserted by the recordset
 ; QUERY RUN TIME: 981201 1.0.73
 ; USING DATABASE F\GAREX\CONFIG_MAIN.mdb
 new data
 ..
<-- end of new data
<-- old data starts here, will be deleted (the marker points to here)
 ; QUERY RUN TIME: 981201 1.0.73
 ; USING DATABASE F\GAREX\CONFIG_MAIN.mdb
 old data
 ..
<-- end of old data
 ; SQL END (this statement triggers deletion)
*/
  if (GetTool().GetId() == ID_TOOL_SQL)
  {
    wxLogVerbose(
      _("File") + ": " + GetFileName().GetFullName() + " Query: " + wxExSkipWhiteSpace(m_SQLQuery));

    Recordset rs(&m_otl.GetConnect(), this);
    if (!rs.ExecQuery(m_SQLQuery))
    {
      return false;
    }

    wxLogVerbose(wxString::Format(_("Retrieved: %d records"), rs.GetRecords());
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
    wxLogError("Missing SQL END after query: " + wxExSkipWhiteSpace(m_SQLQuery));
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

  IncActionsCompleted();

  return true;
}
#endif

void wxExTextFileWithListView::Report(size_t line)
{
  wxASSERT(m_Report != NULL);

  wxExListItem item(m_Report, GetFileName());
  item.Insert();

  item.SetItem(_("Line No"), wxString::Format("%d", line + 1));

  switch (GetTool().GetId())
  {
  case ID_TOOL_REPORT_REPLACE:
    item.SetItem(_("Replaced"), wxExFindReplaceData::Get()->GetReplaceString());
  case ID_TOOL_REPORT_FIND:
    item.SetItem(_("Line"), GetLine(line).Strip(wxString::both));
    item.SetItem(_("Match"), wxExFindReplaceData::Get()->GetFindString());
  break;

  case ID_TOOL_REPORT_REVISION:
    item.SetItem(_("Revision"), GetRCS().GetRevisionNumber());
    item.SetItem(_("Date"), GetRCS().GetRevisionTime().FormatISOCombined(' '));
    item.SetItem(_("Initials"), GetRCS().GetUser());
    item.SetItem(_("Revision Comment"), GetRCS().GetDescription());
  break;

#if wxExUSE_EMBEDDED_SQL
  case ID_TOOL_REPORT_SQL:
    item.SetItem(_("Query"), m_SQLQuery);
    item.SetItem(_("Run Time"), m_SQLQueryRunTime);
  break;
#endif

  default: wxFAIL;
  }
}

void wxExTextFileWithListView::ReportStatistics()
{
  if (GetTool().GetId() == ID_TOOL_REPORT_KEYWORD)
  {
    m_Report = m_Frame->Activate(
      wxExListViewWithFrame::GetTypeTool(GetTool()),
      &GetFileName().GetLexer());

    if (m_Report == NULL)
    {
      return;
    }
  }

  wxExListItem item(m_Report, GetFileName());
  item.Insert();

  switch (GetTool().GetId())
  {
  case ID_TOOL_REPORT_COUNT:
    SetItemColumnStatistics(item, _("Lines"), GetStatistics().GetElements());
    SetItemColumnStatistics(item, _("Lines Of Code"), GetStatistics().GetElements());
    SetItemColumnStatistics(item, _("Empty Lines"), GetStatistics().GetElements());
    SetItemColumnStatistics(item, _("Words Of Code"), GetStatistics().GetElements());
    SetItemColumnStatistics(item, _("Comments"), GetStatistics().GetElements());
    SetItemColumnStatistics(item, _("Comment Size"), GetStatistics().GetElements());
  break;

  case ID_TOOL_REPORT_KEYWORD:
  {
    long total = 0;
    for (size_t i = 0; i < GetFileName().GetLexer().GetKeywords().size(); i++)
    {
      wxListItem col;
      col.SetMask(wxLIST_MASK_TEXT);
      m_Report->GetColumn(i + 1, col);
      const wxString name = col.GetText();
      const wxExStatistics<long>& stat = GetStatistics().GetKeywords();
#ifdef wxExUSE_CPP0X	
      const auto it = stat.GetItems().find(name);
#else
      std::map<wxString,long>::const_iterator it = stat.GetItems().find(name);
#endif	  
      
      if (it != stat.GetItems().end())
      {
        m_Report->SetItem(item.GetId(), i + 1, wxString::Format("%ld", it->second));
        total += it->second;
      }
    }
    
    m_Report->SetItem(
      item.GetId(),
      GetFileName().GetLexer().GetKeywords().size() + 1,
      wxString::Format("%ld", total));
  }
  break;

  default: wxFAIL;
  }
}

#if wxExUSE_EMBEDDED_SQL
bool wxExTextFileWithListView::SetSQLQuery()
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

bool wxExTextFileWithListView::SetupTool(
  const wxExTool& tool, 
  wxExFrameWithHistory* frame)
{
  m_Frame = frame;

#if wxExUSE_EMBEDDED_SQL
  if (tool.GetId() == ID_TOOL_SQL)
  {
    if (m_otl.IsConnected())
    {
      m_otl.Logoff();
    }

    // This is a static function, we cannot use this pointer.
    wxASSERT(wxTheApp != NULL);

    if (!m_otl.Logon(frame))
    {
      return false;
    }
  }
#endif

  if (tool.IsReportType())
  {
    if (tool.GetId() != ID_TOOL_REPORT_KEYWORD)
    {
      m_Report = m_Frame->Activate(wxExListViewWithFrame::GetTypeTool(tool));

      if (m_Report == NULL)
      {
        return false;
      }
    }
  }

  return true;
}

#if wxExUSE_EMBEDDED_SQL
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
    wxLogStatus(p.msg);
    return false;
  }
}

void Recordset::UpdateTextFileFromQuery()
{
  m_TextFile->GoToLine(m_TextFile->GetCurrentLine() + 1);
  WriteComment(QueryRunTimeText() + wxDateTime::Now().FormatISOCombined(' '));
  WriteComment("USING ODBC DATABASE: " + m_otl.Datasource());

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

void Recordset::WriteComment(
  const wxString& text,
  const bool fill_out,
  const bool fill_out_with_space) 
{
  m_TextFile->InsertLine(m_TextFile->GetFileName().GetLexer().MakeComment(
    text,
    fill_out,
    fill_out_with_space));
}
#endif
