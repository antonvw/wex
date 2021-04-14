////////////////////////////////////////////////////////////////////////////////
// Name:      odbc.cpp
// Purpose:   Implementation of wex::odbc class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/core.h>
#include <wex/factory/stc.h>
#include <wex/frame.h>
#include <wex/item-dialog.h>
#include <wex/log.h>
#include <wex/odbc.h>
#include <wx/grid.h>

#if wexUSE_ODBC

#define OTL_CPP_20_ON
#define OTL_DESTRUCTORS_DO_NOT_THROW
#define OTL_ODBC
#define OTL_STL

//#define OTL_UNICODE
#if __UNIX__
#define OTL_ODBC_UNIX
#endif
#include <otlv4.h>

namespace wex
{
  class odbc_imp
  {
  public:
    odbc_imp(bool threaded_mode, size_t buffer_size)
      : m_buffer_size(buffer_size)
    {
      otl_connect::otl_initialize(threaded_mode);
    };

    auto buffer_size() const { return m_buffer_size; };

    auto& connect() { return m_connect; };

  private:
    otl_connect  m_connect;
    const size_t m_buffer_size;
  };

  std::string query_error(const otl_exception& e)
  {
    const std::string query((const char*)e.stm_text);

    if (const std::string str((const char*)e.msg); !str.empty())
    {
      return "OTL error: " + wex::quoted(str);
    }
    else if (const std::string ss((const char*)e.sqlstate); !ss.empty())
    {
      return "sqlstate: " + ss + " in: " + wex::quoted(query);
    }
    else
    {
      return "error, no more info";
    }
  }

  void handle_error(const otl_exception& e, const otl_column_desc& desc)
  {
    wex::log::trace() << query_error(e) << "skipped: (" << desc.otl_var_dbtype
                      << "," << desc.dbsize << ")";
  }
}; // namespace wex

wex::odbc::odbc(bool threaded_mode, size_t buffer_size)
  : m_odbc(std::make_unique<odbc_imp>(threaded_mode, buffer_size))
{
}

wex::odbc::~odbc()
{
  logoff();
}

const std::string wex::odbc::datasource() const
{
  return wex::config(_("Datasource")).get_first_of();
}

bool wex::odbc::logoff()
{
  if (!is_connected())
  {
    return false;
  }

  m_odbc->connect().logoff();

  return true;
}

const wex::version_info wex::odbc::get_version_info()
{
  const long version = OTL_VERSION_NUMBER;

  return version_info({"OTL", version >> 16, 0, (version & 0xffff)});
}

bool wex::odbc::is_connected() const
{
  return m_odbc->connect().connected > 0;
}

bool wex::odbc::logon(const wex::data::window& par)
{
  if (const data::window data(
        data::window(par).title(_("Open ODBC Connection")));
      data.button() != 0)
  {
    if (
      item_dialog(
        {{_("Datasource"),
          item::COMBOBOX,
          std::any(),
          data::control().is_required(true)},
         {_("User")},
         {_("Password"),
          std::string(),
          item::TEXTCTRL,
          data::control().window(data::window().style(wxTE_PASSWORD))}},
        data)
        .ShowModal() == wxID_CANCEL)
    {
      return false;
    }
  }

  try
  {
    const std::string connect = config(_("User")).get() + "/" +
                                config(_("Password")).get() + "@" +
                                datasource();

    m_odbc->connect().rlogon(connect.c_str(),
                             1); // autocommit-flag
  }
  catch (otl_exception& p)
  {
    auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());

    frame->stc_entry_dialog_title("Cannot logon to " + datasource());
    frame->stc_entry_dialog_component()->set_text(
      std::string((const char*)p.msg));
    frame->show_stc_entry_dialog(true);
  }

  return is_connected();
}

long wex::odbc::query(const std::string& query)
{
  if (!is_connected())
  {
    return 0;
  }

  try
  {
    const auto rpc(otl_cursor::direct_exec(m_odbc->connect(), query.c_str()));
    log::trace("query") << query << "records:" << rpc;
    return rpc;
  }
  catch (const otl_exception& e)
  {
    log("query") << query_error(e);
  }

  return -1;
}

// Cannot be const because of open call.
long wex::odbc::query(
  const std::string& query,
  wxGrid*            grid,
  bool&              stopped,
  bool               empty_results)
{
  if (!is_connected())
  {
    return 0;
  }

  assert(grid != nullptr);

  otl_stream i;
  i.set_all_column_types(otl_all_num2str | otl_all_date2str);
  i.open(
    m_odbc->buffer_size(),
    query.c_str(),
    m_odbc->connect(),
    otl_implicit_select);

  long             rows = 0;
  otl_column_desc* desc;

  // Get column names.
  int desc_len;

  try
  {
    desc = i.describe_select(desc_len);
  }
  catch (otl_exception& e)
  {
    log("query") << query_error(e);
    return -1;
  }

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

  log::trace("query grid") << query << "records:" << rows;

  return rows;
}

// Cannot be const because of open call.
long wex::odbc::query(
  const std::string& query,
  wxStyledTextCtrl*  stc,
  bool&              stopped)
{
  if (!is_connected())
  {
    return 0;
  }

  assert(stc != nullptr);

  otl_column_desc* desc;
  otl_stream       i;
  int              desc_len;

  try
  {
    i.set_all_column_types(otl_all_num2str | otl_all_date2str);
    i.open(
      m_odbc->buffer_size(),
      query.c_str(),
      m_odbc->connect(),
      otl_implicit_select);

    stc->NewLine();

    // Get column names.
    desc = i.describe_select(desc_len);

    for (auto n = 0; n < desc_len; n++)
    {
      stc->AppendText(desc[n].name);
      if (n < desc_len - 1)
        stc->AppendText('\t');
    }
  }
  catch (otl_exception& e)
  {
    log("query") << query_error(e);
    return -1;
  }

  stc->NewLine();

  long rows = 0;

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

      if (n < desc_len - 1)
        line += '\t';
    }

    stc->AppendText(line);
    stc->NewLine();

    if ((rows & 0xff) == 0)
    {
      wxTheApp->Yield();
    }

    rows++;
  }

  log::trace("query stc") << query << "records:" << rows;

  return rows;
}
#endif // wexUSE_ODBC
