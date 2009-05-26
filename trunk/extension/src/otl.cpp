/******************************************************************************\
* File:          otl.cpp
* Purpose:       Implementation of otl related things
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/extension/otl.h>
#include <wx/extension/configdialog.h>

#if USE_OTL
bool wxExOTLDialog(
  wxExConfig* config,
  otl_connect* db,
  int max_items)
{
  wxASSERT(db != NULL && config != NULL);

  std::vector<wxExConfigItem> v;

  v.push_back(wxExConfigItem(_("Datasource"),
    CONFIG_COMBOBOX,
    wxEmptyString, // page
    true,          // is_required
    max_items));
  v.push_back(wxExConfigItem(_("User")));
  v.push_back(wxExConfigItem(_("Password"), wxEmptyString, wxTE_PASSWORD));

  // Always show the dialog.
  if (wxExConfigDialog(wxTheApp->GetTopWindow(),
    config,
    v,
    _("Open ODBC Database")).ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  try
  {
    const wxString connect =
      config->Get(_("User")) + "/" +
      config->Get(_("Password")) + "@" +
      config->Get(_("Datasource"));

    db->rlogon(
      connect.c_str(),
      1); // autocommit-flag

    return true;
  }
  catch (otl_exception& p)
  {
    wxLogError("'%s' during opening database: %s",
      p.msg,
      config->Get(_("Datasource")).c_str());
  }

  return false;
}

#if wxUSE_GRID
long wxExOTLQueryToGrid(
  otl_connect* db,
  const wxString& query,
  wxGrid* grid,
  bool& stopped,
  bool empty_results)
{
  wxASSERT(db != NULL && grid != NULL);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    1024,
    query.c_str(),
    *db,
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

    for (int n = 0; n < desc_len; n++)
    {
      grid->SetColLabelValue(n, desc[n].name);
    }

    if (grid->GetNumberRows() > 0)
    {
      grid->DeleteRows(0, grid->GetNumberRows());
    }
  }

  grid->BeginBatch();

  const long startrow = grid->GetNumberRows();

  // Get all rows.
  while (!i.eof() && !stopped)
  {
    grid->AppendRows();

    for (int n = 0; n < desc_len; n++)
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
      if (wxIsMainThread())
      {
        wxTheApp->Yield();
      }
      else
      {
        wxThread::This()->Yield();
      }
    }

    rows++;
  }

  grid->EndBatch();
  grid->AutoSizeColumns();

  return rows;
}
#endif //wxUSE_GRID

long wxExOTLQueryToSTC(
  otl_connect* db,
  const wxString& query,
  wxStyledTextCtrl* stc,
  bool& stopped)
{
  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    1024,
    query.c_str(),
    *db,
    otl_implicit_select);

  stc->AppendText(wxTextFile::GetEOL());

  long rows = 0;
  otl_column_desc* desc;

  // Get column names.
  int desc_len;
  desc = i.describe_select(desc_len);

  for (int n = 0; n < desc_len; n++)
  {
    stc->AppendText(desc[n].name);
    if (n < desc_len - 1) stc->AppendText('\t');
  }

  stc->AppendText(wxTextFile::GetEOL());

  // Get all rows.
  while (!i.eof() && !stopped)
  {
    wxString line;

    for (int n = 0; n < desc_len; n++)
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

    stc->AppendText(line + wxTextFile::GetEOL());

    if ((rows & 0xff) == 0)
    {
      if (wxIsMainThread())
      {
        wxTheApp->Yield();
      }
      else
      {
        wxThread::This()->Yield();
      }
    }

    rows++;
  }

  return rows;
}

const wxString wxExOTLVersion()
{
  const long version = OTL_VERSION_NUMBER;
  return wxString::Format("OTL v%d.%d.%d",
     version >> 16,
    (version >> 8) & 0xff,
    (version & 0xff));
}
#endif // USE_OTL
