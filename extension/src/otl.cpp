////////////////////////////////////////////////////////////////////////////////
// Name:      otl.cpp
// Purpose:   Implementation of wxExOTL class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/grid.h>
#include <wx/stc/stc.h>
#include <wx/extension/otl.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/util.h>

#if wxExUSE_OTL

wxExOTL::wxExOTL(int threaded_mode)
{
  otl_connect::otl_initialize(threaded_mode);
}

wxExOTL::~wxExOTL()
{
  Logoff();
}

const wxString wxExOTL::Datasource() const
{
  return wxExConfigFirstOf(_("Datasource"));
}

bool wxExOTL::Logoff()
{
  if (!IsConnected())
  {
    return false;
  }
  
  m_Connect.logoff();
  
  return true;
}

bool wxExOTL::Logon(
  wxWindow* parent,
  int max_items,
  const wxString& title)
{
  auto* config = wxConfigBase::Get(); 

  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(_("Datasource"),
    CONFIG_COMBOBOX,
    wxEmptyString, // page
    true,          // is_required
    wxID_ANY,
    max_items));
  v.push_back(wxExConfigItem(_("User")));
  v.push_back(wxExConfigItem(_("Password"), 
    wxEmptyString, 
    wxEmptyString,
    wxTE_PASSWORD));

  // Always show the dialog.
  if (wxExConfigDialog(parent,
    v,
    title).ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  try
  {
    const wxString connect =
      config->Read(_("User")) + "/" + config->Read(_("Password")) + "@" +
      Datasource();

    m_Connect.rlogon(
      connect.c_str(),
      1); // autocommit-flag
  }
  catch (otl_exception& p)
  {
    wxLogError("Cannot logon to %s because of: %s",
      Datasource().c_str(), wxString(p.msg).c_str());
  }

  return IsConnected();
}

long wxExOTL::Query(const wxString& query)
{
  return otl_cursor::direct_exec(m_Connect, query.c_str());
}

#if wxUSE_GRID
// Cannot be const because of open call.
long wxExOTL::Query(
  const wxString& query,
  wxGrid* grid,
  bool& stopped,
  bool empty_results,
  int buffer_size)
{
  wxASSERT(grid != NULL);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    buffer_size,
    query.c_str(),
    m_Connect,
    otl_implicit_select);

  long rows = 0;
  otl_column_desc* desc;

  // Get column names.
  int desc_len;
  desc = i.describe_select(desc_len);

  if (empty_results)
  {
    if (grid->GetNumberCols() > 0)
    {
      grid->DeleteCols(0, grid->GetNumberCols());
    }

    grid->AppendCols(desc_len);

    for (auto n = 0; n < desc_len; n++)
    {
      grid->SetColLabelValue(n, desc[n].name);
    }

    if (grid->GetNumberRows() > 0)
    {
      grid->DeleteRows(0, grid->GetNumberRows());
    }
  }

  grid->BeginBatch();

  const auto startrow = grid->GetNumberRows();

  // Get all rows.
  while (!i.eof() && !stopped)
  {
    grid->AppendRows();

    for (auto n = 0; n < desc_len; n++)
    {
      try
      {
        if (desc[n].otl_var_dbtype == otl_var_varchar_long)
        {
          otl_long_string var;
          i >> var;
          grid->SetCellValue(startrow + rows, n, var.v);
        }
        else
        {
          std::string s;
          i >> s;
          grid->SetCellValue(startrow + rows, n, s);
        }
      }
      catch (otl_exception&)
      {
        // Ignore error.
        grid->SetCellValue(startrow + rows, n, _("<Skipped>"));
      }
    }

    if ((rows & 0xff) == 0)
    {
      wxTheApp->Yield();
    }

    rows++;
  }

  grid->EndBatch();
  grid->AutoSizeColumns(false); // not set as minimum width

  return rows;
}
#endif //wxUSE_GRID

// Cannot be const because of open call.
long wxExOTL::Query(
  const wxString& query,
  wxStyledTextCtrl* stc,
  bool& stopped,
  int buffer_size)
{
  wxASSERT(stc != NULL);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    buffer_size,
    query.c_str(),
    m_Connect,
    otl_implicit_select);

  stc->NewLine();

  long rows = 0;
  otl_column_desc* desc;

  // Get column names.
  int desc_len;
  desc = i.describe_select(desc_len);

  for (auto n = 0; n < desc_len; n++)
  {
    stc->AppendText(desc[n].name);
    if (n < desc_len - 1) stc->AppendText('\t');
  }

  stc->NewLine();

  // Get all rows.
  while (!i.eof() && !stopped)
  {
    wxString line;

    for (auto n = 0; n < desc_len; n++)
    {
      try
      {
        if (desc[n].otl_var_dbtype == otl_var_varchar_long)
        {
          otl_long_string var;
          i >> var;
          line += var.v;
        }
        else
        {
          std::string s;
          i >> s;
          line += s;
        }
      }
      catch (otl_exception&)
      {
        // Ignore error.
        line += _("<Skipped>");
        line += wxString::Format(" (%d, %d)",
          desc[n].otl_var_dbtype, desc[n].dbsize);
      }

      if (n < desc_len - 1) line += '\t';
    }

    stc->AppendText(line);
    stc->NewLine();
    
    if ((rows & 0xff) == 0)
    {
      wxTheApp->Yield();
    }

    rows++;
  }

  return rows;
}

const wxVersionInfo wxExOTL::VersionInfo()
{
  const long version = OTL_VERSION_NUMBER;

  return wxVersionInfo(
    "OTL", 
     version >> 16,
     0,
    (version & 0xffff));
}
#endif // wxExUSE_OTL
