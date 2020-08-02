////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "app.h"
#include <wex/bind.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/grid.h>
#include <wex/log.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wx/numdlg.h>

#ifndef __WXMSW__
#include "app.xpm"
#include "connect.xpm"
#include "notready.xpm"
#include "ready.xpm"
#endif

#ifdef __WXOSX__
#undef wxUSE_TASKBARICON
#endif

#ifdef __WXGTK__
#undef wxUSE_TASKBARICON
#endif

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("syncsocketserver");

  if (wex::data::cmdline data(argc, argv);
      !wex::app::OnInit() || !wex::cmdline().parse(data))
  {
    return false;
  }

  new frame;

  return true;
}

frame::frame()
  : wex::report::frame()
  , m_timer(this)
  , m_data(new wex::stc)
  , m_log(new wex::stc(
      std::string(),
      wex::data::stc().flags(
        wex::data::stc::window_t().set(wex::data::stc::WIN_NO_INDICATOR))))
  , m_shell(new wex::shell)
  , m_id_clear_log(NewControlId())
  , m_id_socket_client(NewControlId())
  , m_id_socket_server(NewControlId())
  , m_id_clear_statistics(NewControlId())
  , m_id_client_buffer_size(NewControlId())
  , m_id_client_answer_command(NewControlId())
  , m_id_client_answer_echo(NewControlId())
  , m_id_client_answer_file(NewControlId())
  , m_id_client_answer_off(NewControlId())
  , m_id_client_log_data(NewControlId())
  , m_id_client_log_data_count_only(NewControlId())
  , m_id_hide(NewControlId())
  , m_id_recent_file_menu(NewControlId())
  , m_id_server_config(NewControlId())
  , m_id_remote_server_connect(NewControlId())
  , m_id_remote_server_disconnect(NewControlId())
  , m_id_remote_server_config(NewControlId())
  , m_id_socket_remoteclient(NewControlId())
  , m_id_timer_stop(NewControlId())
  , m_id_timer_start(NewControlId())
  , m_id_view_data(NewControlId())
  , m_id_view_log(NewControlId())
  , m_id_view_shell(NewControlId())
  , m_id_view_statistics(NewControlId())
  , m_id_write_data(NewControlId())
{
  SetIcon(wxICON(app));

#if wxUSE_TASKBARICON
  m_taskbar_icon = new taskbar_icon(this);
#endif

  Show(); // otherwise statusbar is not placed correctly

  setup_statusbar({wex::statusbar_pane("PaneConnections", 75)
                     .help("Number of local, remote connections"),
                   wex::statusbar_pane("PaneTimer", 75).help("Repeat timer"),
                   wex::statusbar_pane("PaneBytes", 150)
                     .help("Number of bytes received and sent"),
                   {"PaneFileType", 50},
                   {"PaneInfo", 100},
                   {"PaneMode", 100, false}});

  m_log->reset_margins();

  build_menu();

  if (setup_server())
  {
#if wxUSE_TASKBARICON
    Hide();
#endif
  }

  if (!file_history().get_history_file().empty() && pane_is_shown("DATA"))
  {
    open_file(file_history().get_history_file());
  }

  if (pane_is_shown("SHELL"))
  {
    m_shell->SetFocus();
    m_shell->DocumentEnd();
  }

  get_toolbar()->add_standard(false); // no realize yet
  get_toolbar()->add_tool({wex::data::toolbar_item(m_id_write_data)
                             .bitmap(wxArtProvider::GetBitmap(
                               wxART_GO_FORWARD,
                               wxART_TOOLBAR,
                               get_toolbar()->GetToolBitmapSize()))
                             .label("Write data"),
                           wex::data::toolbar_item(m_id_clear_log)
                             .bitmap(wxArtProvider::GetBitmap(
                               wxART_CROSS_MARK,
                               wxART_TOOLBAR,
                               get_toolbar()->GetToolBitmapSize()))
                             .label("Clear log")});

  get_find_toolbar()->add_find();
  get_options_toolbar()->add_checkboxes_standard();

  pane_add(
    {{m_log,
      wxAuiPaneInfo().CenterPane().MaximizeButton(true).Caption("Log").Name(
        "LOG")},
     {m_data,
      wxAuiPaneInfo().Hide().Left().MaximizeButton(true).Caption("Data").Name(
        "DATA")},
     {m_shell,
      wxAuiPaneInfo().Hide().Left().MaximizeButton(true).Caption("Shell").Name(
        "SHELL")},
     {m_stats.show(this),
      wxAuiPaneInfo()
        .Hide()
        .Left()
        .MaximizeButton(true)
        .Caption("Statistics")
        .Name("STATISTICS")}},
    "Perspective");

  bind_all();
}

frame::~frame()
{
#if wxUSE_TASKBARICON
  delete m_taskbar_icon;
#endif
  delete m_client;
  delete m_server;
}

void frame::append_text(
  wex::stc*          stc,
  const std::string& text,
  data_mode_t        mode)
{
  const bool pos_at_end = (stc->GetCurrentPos() == stc->GetTextLength());

  std::string prefix;

  switch (mode)
  {
    case data_mode_t::MESSAGE:
      break;
    case data_mode_t::MESSAGE_RAW:
      break;
    case data_mode_t::READ:
      prefix = "r: ";
      break;
    case data_mode_t::WRITE:
      prefix = "w: ";
      break;
  }

  if (mode != data_mode_t::MESSAGE_RAW)
  {
    stc->AppendText(wxDateTime::Now().Format("%T") + " " + prefix);
  }

  if (
    !stc->is_hexmode() || mode == data_mode_t::MESSAGE ||
    mode == data_mode_t::MESSAGE_RAW)
  {
    stc->AppendText(text);
  }
  else
  {
    stc->get_hexmode().append_text(text);
  }

  if (text.back() != '\n')
  {
    stc->AppendText(stc->eol());
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();

  if (pos_at_end)
  {
    stc->DocumentEnd();
  }
}

void frame::log_connection(
  wxSocketBase* sock,
  bool          opend,
  bool          show_connections)
{
  std::stringstream text;
  text << (opend ? "opened" : "closed") << " " << socket_details(sock);

  const auto connections =
    m_clients.size() + (m_client != nullptr && m_client->IsConnected() ? 1 : 0);

  if (show_connections && connections > 0)
  {
    text << " "
         << "connections: " << connections;
  }

  append_text(m_log, text.str(), data_mode_t::MESSAGE);
  wex::log::verbose("connection") << text;

  update_connections_pane();
}

void frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (event.GetId() != wxID_CANCEL)
    {
      m_data->config_get();
      m_log->config_get();
      m_shell->config_get();
    }
  }
  else
  {
    wex::report::frame::on_command_item_dialog(dialogid, event);
  }
}

wex::stc*
frame::open_file(const wex::path& filename, const wex::data::stc& data)
{
  if (m_data->open(filename, data))
  {
    pane_show("DATA");
  }

  return m_data;
}

bool frame::setup_server()
{
  assert(m_server == nullptr);

  // Create the address - defaults to localhost and port as specified
  wxIPV4address addr;
  addr.Hostname(wex::config("Hostname").get("localhost"));
  addr.Service(wex::config("Port").get(3000));

  if (wxSocketServer* server = new wxSocketServer(addr); !server->Ok())
  {
    const std::string text(
      "could not listen at " + std::to_string(wex::config("Port").get(3000)));

#if wxUSE_TASKBARICON
    m_taskbar_icon->SetIcon(wxICON(notready), text);
#endif

    server->Destroy();

    wex::log::status(text);
    wex::log() << text;

    return false;
  }
  else
  {
    const std::string text(
      "server listening at " + std::to_string(wex::config("Port").get(3000)));

#if wxUSE_TASKBARICON
    m_taskbar_icon->SetIcon(wxICON(ready), text);
#endif

    wex::log::verbose("server")
      << "listening at" << wex::config("Port").get(3000);
    wex::log::status(text);
    append_text(m_log, text, data_mode_t::MESSAGE);

    // Setup the event handler and subscribe to connection events
    m_server = server;
    m_server->SetEventHandler(*this, m_id_socket_server);
    m_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_server->Notify(true);

    return true;
  }
}

void frame::socket_closed(wxSocketBase* sock, bool remove_from_clients)
{
  assert(sock != nullptr);

  if (remove_from_clients)
  {
    m_clients.remove(sock);
  }

  log_connection(sock, false, remove_from_clients);

  sock->Destroy();
}

const std::string frame::socket_details(const wxSocketBase* sock) const
{
  assert(sock != nullptr);

  std::stringstream text;

  if (wxIPV4address peer_addr, local_addr; !sock->GetPeer(peer_addr))
  {
    wex::log() << "could not get peer address";
  }
  else if (!sock->GetLocal(local_addr))
  {
    wex::log() << "could not get local address";
  }
  else
  {
    text << "socket: " << local_addr.IPAddress() << "." << local_addr.Service()
         << ", " << peer_addr.IPAddress() << "." << peer_addr.Service();
  }

  return text.str();
}

void frame::statusbar_clicked(const std::string& pane)
{
  if (pane == "PaneTimer")
  {
    timer_dialog();
  }
  else
  {
    wex::report::frame::statusbar_clicked(pane);
  }
}

void frame::timer_dialog()
{
  const long val = wxGetNumberFromUser(
    "Input (seconds):",
    wxEmptyString,
    "Repeat Timer",
    wex::config("Timer").get(60),
    1,
    3600 * 24);

  // If cancelled, -1 is returned.
  if (val == -1)
    return;

  wex::config("Timer").set(val);

  if (val > 0)
  {
    m_timer.Start(1000 * val);
    wex::log::verbose("timer")
      << "set" << val << "seconds"
      << wxTimeSpan(0, 0, val, 0).Format().ToStdString();
    statustext(std::to_string(val), "PaneTimer");
  }
  else if (val == 0)
  {
    m_timer.Stop();
    wex::log::verbose("timer") << "stopped";
    statustext(std::string(), "PaneTimer");
  }
}

void frame::update_connections_pane() const
{
  statustext(
    std::to_string(m_clients.size()) + "," +
      std::to_string(m_client != nullptr && m_client->IsConnected() ? 1 : 0),
    "PaneConnections");
}

#if wxUSE_TASKBARICON
void frame::update_taskbar()
{
  if (m_clients.empty())
  {
    m_taskbar_icon->SetIcon(
      wxICON(ready),
      "server listening at " + std::string(wex::config("Port").get(3000)));
  }
  else
  {
    std::stringstream ss;
    ss << 
      wxTheApp->GetAppName().c_str() << " " <<
      m_clients.size() +  
          (m_client != nullptr && m_client->IsConnected() ? 1: 0) <<
      " connections connected at " << std::to_string(wex::config("Port").get(3000)) << 
      "\nbytes received: " + std::to_string(m_stats.get("Bytes Received")) <<
      "\nbytes sent: " + std::to_string(m_stats.get("Bytes Sent")));

    m_taskbar_icon->SetIcon(wxICON(connect), ss.str());
  }
}
#endif

size_t
frame::write_data_to_socket(const std::string& buffer, wxSocketBase* sock)
{
  if (buffer.empty())
    return 0;

  size_t written = 0;
  bool   error   = false;

  while (written < buffer.size() && !error)
  {
    if (sock->Write(buffer.data() + written, buffer.length() - written).Error())
    {
      m_stats.inc("Socket Error");
      error = true;
    }

    written += sock->LastWriteCount();
  }

  m_stats.inc("Bytes Sent", written);
  m_stats.inc("Messages Sent");

  statustext(
    std::to_string(m_stats.get("Bytes Received")) + "," +
      std::to_string(m_stats.get("Bytes Sent")),
    "PaneBytes");

#if wxUSE_TASKBARICON
  update_taskbar();
#endif

  if (wex::config("Log Data").get(true))
  {
    if (wex::config("Count Only").get(true))
    {
      append_text(
        m_log,
        "write " + std::to_string(written) + " bytes to " +
          socket_details(sock),
        data_mode_t::MESSAGE);
    }
    else
    {
      append_text(m_log, buffer, data_mode_t::WRITE);
    }
  }

  if (error)
    wex::log("write") << std::to_string(sock->LastError()) << "at:" << written;
  else
    wex::log::verbose("write") << buffer;

  return written;
}

size_t frame::write_data_window_to_connections()
{
  size_t written = 0;

  if (const auto& buffer(m_data->get_text()); !buffer.empty())
  {
    for (auto& it : m_clients)
    {
      written += write_data_to_socket(buffer, it);
    }

    if (m_client != nullptr && m_client->IsConnected())
    {
      written += write_data_to_socket(buffer, m_client);
    }
  }

  return written;
}

const auto m_id_open = wxWindow::NewControlId();

#if wxUSE_TASKBARICON
taskbar_icon::taskbar_icon(frame* frame)
  : m_frame(frame)
{
  bind(this).command({{[=](wxCommandEvent& event) {
                         m_frame->Close(true);
                       },
                       wxID_EXIT},
                      {[=](wxCommandEvent& event) {
                         m_frame->Show();
                       },
                       m_id_open}});

  Bind(wxEVT_TASKBAR_LEFT_DCLICK, [=](wxTaskBarIconEvent&) {
    m_frame->Show();
  });

  Bind(
    wxEVT_UPDATE_UI,
    [=](wxUpdateUIEvent& event) {
      event.Enable(m_frame->server_not_listening());
    },
    wxID_EXIT);
}

wxMenu* taskbar_icon::CreatePopupMenu()
{
  return new wex::menu({{m_id_open, "Open"}, {}, {wxID_EXIT}});
}
#endif
