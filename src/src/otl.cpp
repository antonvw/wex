////////////////////////////////////////////////////////////////////////////////
// Name:      otl.cpp
// Purpose:   Implementation of wex::otl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/grid.h>
#include <wx/stc/stc.h>
#include <wex/otl.h>
#include <wex/config.h>
#include <wex/itemdlg.h>
#include <wex/log.h>
#include <wex/stcdlg.h>
#include <wex/util.h>

#if wexUSE_OTL

wex::otl::otl(int threaded_mode)
{
  otl_connect::otl_initialize(threaded_mode);
}

wex::otl::~otl()
{
  logoff();
}

const std::string wex::otl::datasource() const
{
  return wex::config(_("Datasource")).firstof();
}

bool wex::otl::logoff()
{
  if (!is_connected())
  {
    return false;
  }
  
  m_connect.logoff();
  
  return true;
}

const wex::version_info wex::otl::get_version_info()
{
  const long version = OTL_VERSION_NUMBER;

  return version_info(
    "OTL", 
     version >> 16,
     0,
    (version & 0xffff));
}

void wex::otl::handle_error(
  const otl_exception& e, const otl_column_desc& desc) const
{
  log::verbose() <<
    "OTL error: " << (const char *)e.msg <<
    "skipped: (" << desc.otl_var_dbtype << "," << desc.dbsize << ")";
}
  
bool wex::otl::logon(const wex::window_data& par)
{
  const window_data data(window_data(par).
    title(_("Open ODBC Connection")));
    
  if (data.button() != 0)
  {
    if (item_dialog({
        {_("Datasource"), item::COMBOBOX, std::any(), control_data().is_required(true)},
        {_("User")},
        {_("Password"), std::string(), item::TEXTCTRL, control_data().window(window_data().style(wxTE_PASSWORD))}}, 
        data).ShowModal() == wxID_CANCEL)
    {
      return false;
    }
  }

  try
  {
    const std::string connect =
      config(_("User")).get() + "/" + 
      config(_("Password")).get() + "@" +
      datasource();

    m_connect.rlogon(connect.c_str(),
      1); // autocommit-flag
  }
  catch (otl_exception& p)
  {
    stc_entry_dialog(
      "Cannot logon to " + datasource() + 
      " because of: " + std::string((const char *)p.msg)).Show();
  }

  return is_connected();
}

long wex::otl::query(const std::string& query)
{
  if (!is_connected())
  {
    return 0;
  }
  
  const auto records(otl_cursor::direct_exec(m_connect, query.c_str()));
  log::verbose("query") << query << "records: " << records;
  return records;
}

// Cannot be const because of open call.
long wex::otl::query(
  const std::string& query,
  wxGrid* grid,
  bool& stopped,
  bool empty_results,
  int buffer_size)
{
  if (!is_connected())
  {
    return 0;
  }
  
  assert(grid != nullptr);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    buffer_size,
    query.c_str(),
    m_connect,
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
      catch (otl_exception& e)
      {
        grid->SetCellValue(startrow + rows, n, _("<Skipped>"));
        handle_error(e, desc[n]);
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

  log::verbose("query grid") << query << "records: " << rows;

  return rows;
}

// Cannot be const because of open call.
long wex::otl::query(
  const std::string& query,
  wxStyledTextCtrl* stc,
  bool& stopped,
  int buffer_size)
{
  if (!is_connected())
  {
    return 0;
  }
  
  assert(stc != nullptr);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    buffer_size,
    query.c_str(),
    m_connect,
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
      catch (otl_exception& e)
      {
        handle_error(e, desc[n]);
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

  log::verbose("queryi stc") << query << "records: " << rows;

  return rows;
}
#endif // wex::use_OTL
