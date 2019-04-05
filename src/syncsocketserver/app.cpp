////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <algorithm>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include <wex/config.h>
#include <wex/filedlg.h>
#include <wex/grid.h>
#include <wex/itemdlg.h>
#include <wex/lexers.h>
#include <wex/statistics.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/version.h>
#include "app.h"

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

#if wxUSE_SOCKETS
enum
{
  ANSWER_OFF,
  ANSWER_ECHO,
  ANSWER_COMMAND,
  ANSWER_FILE,
};

enum
{
  DATA_MESSAGE,
  DATA_MESSAGE_RAW,
  DATA_READ,
  DATA_WRITE,
};

const auto id_clear_log = wxWindow::NewControlId();
const auto id_clear_statistics = wxWindow::NewControlId();
const auto id_client_buffer_size  = wxWindow::NewControlId();
const auto id_client_answer_command = wxWindow::NewControlId();
const auto id_client_answer_echo = wxWindow::NewControlId();
const auto id_client_answer_file = wxWindow::NewControlId();
const auto id_client_answer_off = wxWindow::NewControlId();
const auto id_client_log_data = wxWindow::NewControlId();
const auto id_client_log_data_count_only = wxWindow::NewControlId();
const auto id_hide = wxWindow::NewControlId();
const auto id_recent_file_menu = wxWindow::NewControlId();
const auto id_server_config = wxWindow::NewControlId();
const auto id_remote_server_connect = wxWindow::NewControlId();
const auto id_remote_server_disconnect = wxWindow::NewControlId();
const auto id_remote_server_config = wxWindow::NewControlId();
const auto id_socket_server = wxWindow::NewControlId();
const auto id_socket_remoteclient = wxWindow::NewControlId();
const auto id_socket_client = wxWindow::NewControlId();
const auto id_timer_stop = wxWindow::NewControlId();
const auto id_timer_start = wxWindow::NewControlId();
const auto id_view_data = wxWindow::NewControlId();
const auto id_view_log = wxWindow::NewControlId();
const auto id_view_shell  = wxWindow::NewControlId();
const auto id_view_statistics = wxWindow::NewControlId();
const auto id_write_data = wxWindow::NewControlId();

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("syncsocketserver");

  if (!wex::app::OnInit())
  {
    return false;
  }

  new frame;

  return true;
}

frame::frame()
  : wex::report::frame()
  , m_Timer(this)
  , m_Answer(ANSWER_OFF)
  , m_DataWindow(new wex::stc)
  , m_LogWindow(new wex::stc(
      std::string(), wex::stc_data().flags(
        wex::stc_data::window_t().set(wex::stc_data::WIN_NO_INDICATOR))))
  , m_Shell(new wex::shell)
{
  SetIcon(wxICON(app));

#if wxUSE_TASKBARICON
  m_TaskBarIcon = new taskbar_icon(this);
#endif

  Show(); // otherwise statusbar is not placed correctly

  // Statusbar setup before STC construction.
  setup_statusbar({
    {"PaneConnections", 75, _("Number of local, remote connections").ToStdString()},
    {"PaneTimer", 75, _("Repeat timer").ToStdString()},
    {"PaneBytes", 150, _("Number of bytes received and sent").ToStdString()},
    {"PaneFileType", 50, _("File type").ToStdString()},
    {"PaneTheme", 50, _("Theme").ToStdString()},
    {"PaneInfo", 100, _("Lines").ToStdString()},
    {"PaneMode", 100}});

  if (wex::lexers::get()->get_themes_size() <= 1)
  {
    m_StatusBar->show_field("PaneTheme", false);
  }

  m_LogWindow->reset_margins();

  auto* menuFile = new wex::menu();
  menuFile->append(wxID_NEW);
  menuFile->append(wxID_OPEN);
  file_history().use_menu(id_recent_file_menu, menuFile);
  menuFile->append_separator();
  menuFile->append(wxID_SAVE);
  menuFile->append(wxID_SAVEAS);
  menuFile->append_separator();
  menuFile->append(id_clear_statistics, 
    _("Clear Statistics"), _("Clears the statistics"));
  menuFile->append_separator();
#if wxUSE_TASKBARICON
  menuFile->append(id_hide, _("Hide"), _("Puts back in the task bar"));
#else
  menuFile->append(wxID_EXIT);
#endif

  auto* menuRemote = new wex::menu();
  menuRemote->append(id_remote_server_config, _("Configuration"), 
    _("Configures the remote server"));
  menuRemote->append_separator();
  menuRemote->append(id_remote_server_connect, 
    _("Connect"), _("Tries to connect to server"));
  menuRemote->append(id_remote_server_disconnect, 
    _("Disconnect"), _("Disconnects from remote server"));

  auto* menuServer = new wex::menu();
  menuServer->append(id_server_config, _("Configuration"), 
    _("Configures the server"));
  menuServer->append_separator();
  menuServer->append(wxID_EXECUTE);
  menuServer->append(wxID_STOP);
  menuServer->append_separator();
  menuServer->AppendSubMenu(menuRemote, _("&Remote"));

  auto* menuAnswer = new wxMenu();
  menuAnswer->AppendRadioItem(id_client_answer_off, _("Off"),
    _("No answer back to connection"));
  menuAnswer->AppendRadioItem(id_client_answer_echo, _("Echo"),
    _("Echo's received data back to connection"));
  menuAnswer->AppendRadioItem(id_client_answer_command, _("Command"),
    _("Send last shell command back to connection"));
  menuAnswer->AppendRadioItem(id_client_answer_file, _("File"),
    _("Send file contents back to connection"));
    
  auto* menuConnection = new wex::menu();
  menuConnection->append_submenu(menuAnswer, _("&Answer"));
  menuConnection->append_separator();
  menuConnection->AppendCheckItem(id_client_log_data, _("Log Data"),
    _("Logs data read from and written to connection"));
  menuConnection->AppendCheckItem(id_client_log_data_count_only, _("Count Only"),
    _("Logs only byte counts, no text"));
  menuConnection->append_separator();
  menuConnection->append(id_client_buffer_size, wex::ellipsed(_("Buffer Size")),
    _("Sets buffersize for data retrieved from connection"));
  menuConnection->append_separator();
  menuConnection->append(id_timer_start, wex::ellipsed(_("Repeat Timer")),
    _("Repeats with timer writing last data to all connections"));
  menuConnection->append(id_timer_stop, _("Stop Timer"), _("Stops the timer"));
  menuConnection->append_separator();
  menuConnection->append(id_write_data, _("Write"), 
    _("Writes data to all connections"), wxART_GO_FORWARD);

  auto* menuView = new wex::menu();
  append_panes(menuView);
  menuView->append_separator();
  menuView->AppendCheckItem(id_view_log, _("Log"));
  menuView->AppendCheckItem(id_view_data, _("Data"));
  menuView->AppendCheckItem(id_view_shell, _("Shell"));
  menuView->AppendCheckItem(id_view_statistics, _("Statistics"));

#ifndef __WXOSX__
  auto* menuOptions = new wex::menu();
  menuOptions->append(wxID_PREFERENCES);
#else
  menuFile->append(wxID_PREFERENCES); // is moved!
#endif

  auto* menuHelp = new wex::menu();
  menuHelp->append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menuBar->Append(menuView, _("&View"));
  menuBar->Append(menuServer, _("&Server"));
  menuBar->Append(menuConnection, _("&Connection"));
#ifndef __WXOSX__
  menuBar->Append(menuOptions, _("&Options"));
#endif
  menuBar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  SetMenuBar(menuBar);

  manager().AddPane(m_LogWindow,
    wxAuiPaneInfo().CenterPane().MaximizeButton(true).Caption(_("Log")).
      Name("LOG"));
  manager().AddPane(m_DataWindow,
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Data")).
      Name("DATA"));
  manager().AddPane(m_Shell,
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Shell")).
      Name("SHELL"));
  manager().AddPane(m_Statistics.show(this),
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Statistics")).
      Name("STATISTICS"));

  manager().LoadPerspective(wex::config("Perspective").get());

  if (SetupSocketServer())
  {
#if wxUSE_TASKBARICON
    Hide();
#endif
  }

  if (!file_history().get_history_file().data().empty() && manager().GetPane("DATA").IsShown())
  {
    open_file(file_history().get_history_file());
  }

  if (manager().GetPane("SHELL").IsShown())
  {
    m_Shell->SetFocus();
    m_Shell->DocumentEnd();
  }

  get_toolbar()->add_controls(false); // no realize yet

  get_toolbar()->add_tool(
    id_write_data,
    wxEmptyString,
    wxArtProvider::GetBitmap(
      wxART_GO_FORWARD, wxART_TOOLBAR, get_toolbar()->GetToolBitmapSize()),
    _("Write data"));
  get_toolbar()->add_tool(
    id_clear_log,
    wxEmptyString,
    wxArtProvider::GetBitmap(
      wxART_CROSS_MARK, wxART_TOOLBAR, get_toolbar()->GetToolBitmapSize()),
    _("Clear log"));

  get_toolbar()->Realize();
    
  get_options_toolbar()->add_controls();
  
  manager().Update();
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general socket server."));
    info.SetVersion(wex::get_version_info().get());
    info.SetCopyright(wex::get_version_info().copyright());
    wxAboutBox(info);
    }, wxID_ABOUT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    SetupSocketServer();}, wxID_EXECUTE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {;}, wxID_HELP);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_DataWindow->get_file().file_new(wex::path());
    show_pane("DATA");}, wxID_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::open_files_dialog(
      this, 
      wxFD_OPEN | wxFD_CHANGE_DIR, 
      wxFileSelectorDefaultWildcardStr, 
      true);}, wxID_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_DataWindow->get_file().file_save();}, wxID_SAVE);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::file_dialog dlg(
      &m_DataWindow->get_file(), 
      wex::window_data().
        style(wxFD_SAVE).
        parent(this).
        title(wxGetStockLabel(wxID_SAVEAS).ToStdString()));
    if (dlg.ShowModal())
    {
      m_DataWindow->get_file().file_save(dlg.GetPath().ToStdString());
    }}, wxID_SAVEAS);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (wxMessageBox(_("Stop server?"),
      _("Confirm"),
      wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
    {
      return;
    }

    const wxString text = _("server stopped");
    wex::log::status(text);
    AppendText(m_LogWindow, text, DATA_MESSAGE);

    for (auto& it : m_Clients)
    {
      SocketClosed(it, false);
    }

    m_Clients.clear();
    m_SocketServer->Destroy();
    m_SocketServer = nullptr;

#if wxUSE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(notready), text);
#endif
    UpdateConnectionsPane();
    }, wxID_STOP);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Statistics.clear();}, id_clear_statistics);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Answer = ANSWER_COMMAND;}, id_client_answer_command);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Answer = ANSWER_ECHO;}, id_client_answer_echo);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) { 
    m_Answer = ANSWER_FILE;}, id_client_answer_file);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Answer = ANSWER_OFF;}, id_client_answer_off);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    long val;
    if ((val = wxGetNumberFromUser(
      _("Input") + ":",
      wxEmptyString,
      _("Buffer Size"),
      wex::config(_("Buffer Size")).get(4096),
      1,
      65536)) > 0)
      {
        wex::config(_("Buffer Size")).set(val);
      }
    }, id_client_buffer_size);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::config(_("Log Data")).set( 
      !wex::config(_("Log Data")).get(true));}, id_client_log_data);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::config(_("Count Only")).set(
      !wex::config(_("Count Only")).get(true));}, id_client_log_data_count_only);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(false);}, id_hide);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // Configuring only possible if no client is active,
    // otherwise just show settings readonly mode.
    wex::item_dialog({
        {_("Remote Hostname"), wex::item::COMBOBOX, std::any(), wex::control_data().is_required(true)},
        // Well known ports are in the range from 0 to 1023.
        // Just allow here for most flexibility.
        {_("Remote Port"), 1, 65536}},
      wex::window_data().
        title(_("Remote Server Config").ToStdString()).
        button(m_SocketRemoteClient == nullptr ? wxOK | wxCANCEL: wxCANCEL)).ShowModal();
    }, id_remote_server_config);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_SocketRemoteClient == nullptr) return;

    if (m_SocketRemoteClient->IsConnected())
    {
      SocketClosed(m_SocketRemoteClient, false);
    }

    UpdateConnectionsPane();

    m_SocketRemoteClient = nullptr;
    }, id_remote_server_disconnect);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_SocketRemoteClient != nullptr)
    {
      // In fact, this should not happen, 
      // but using Ubuntu the OnUpdateUI does not prevent this...
      wex::log::status(_("First stop remote server"));
      return;
    }
    
    // Create the address - defaults to localhost and port as specified
    if (!wex::config(_("Remote Hostname")).exists())
    {
      wex::config::set_record_defaults(true);
    }

    wxIPV4address addr;
    addr.Hostname(wex::config(_("Remote Hostname")).get("localhost"));
    addr.Service(wex::config(_("Remote Port")).get(3000));
    wex::config::set_record_defaults(false);

    m_SocketRemoteClient = new wxSocketClient();

    m_SocketRemoteClient->SetEventHandler(*this, id_socket_remoteclient);
    m_SocketRemoteClient->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_SocketRemoteClient->Notify(true);
    m_SocketRemoteClient->Connect(addr, false);
    }, id_remote_server_connect);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    // Configuring only possible if server is stopped,
    // otherwise just show settings readonly mode.
    wex::item_dialog({
        {_("Hostname"), std::string(), wex::item::TEXTCTRL, wex::control_data().is_required(true)},
        // Well known ports are in the range from 0 to 1023.
        // Just allow here for most flexibility.
        {_("Port"), 1, 65536}},
      wex::window_data().
        title(_("Server Config").ToStdString()).
        button(m_SocketServer == nullptr ? wxOK | wxCANCEL: wxCANCEL)).ShowModal();
    }, id_server_config);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const wxString str = event.GetString() + "\n";
    const wxCharBuffer& buffer(str.c_str());
    for (auto& it : m_Clients)
    {
      WriteDataToSocket(buffer, it);
    }
    if (m_SocketRemoteClient != nullptr)
    {
      WriteDataToSocket(buffer, m_SocketRemoteClient);
    }

    m_Shell->prompt();}, wex::ID_SHELL_COMMAND);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TimerDialog();}, id_timer_start);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Timer.Stop();
    AppendText(m_LogWindow, _("timer stopped"), DATA_MESSAGE);
    statustext(std::string(), "PaneTimer");
   }, id_timer_stop);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("DATA");}, id_view_data);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("LOG");}, id_view_log);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("SHELL");}, id_view_shell);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    toggle_pane("STATISTICS");}, id_view_statistics);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    WriteDataWindowToConnections();}, id_write_data);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_LogWindow->clear();}, id_clear_log);

  Bind(wxEVT_SOCKET, [=](wxSocketEvent& event) {
    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).
    wxSocketBase*sock = m_SocketServer->Accept(false);

    if (sock == nullptr)
    {
      AppendText(m_LogWindow,
        _("error: couldn't accept a new connection"),
        DATA_MESSAGE);
    }
    else
    {
      m_Statistics.inc("Connections Server");
      sock->SetEventHandler(*this, id_socket_client);
      sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
      sock->Notify(true);
      m_Clients.emplace_back(sock);
      LogConnection(sock, true);
      const wxCharBuffer& buffer = m_DataWindow->GetTextRaw();
      WriteDataToSocket(buffer, sock);
#if wxUSE_TASKBARICON
      UpdateTaskBar();
#endif
    };}, id_socket_server);

  Bind(wxEVT_SOCKET, [=](wxSocketEvent& event) {
    wxSocketBase *sock = event.GetSocket();

    switch (event.GetSocketEvent())
    {
      case wxSOCKET_CONNECTION:
        {
        m_Statistics.inc("Connections Remote");
        LogConnection(sock, true);
        sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG);
        const wxCharBuffer& buffer = m_DataWindow->GetTextRaw();
        WriteDataToSocket(buffer, sock);
        }
        break;

      case wxSOCKET_INPUT:
      case wxSOCKET_OUTPUT:
        {
        // We disable input events, so that the test doesn't trigger
        // wxSocketEvent again.
        sock->SetNotify(wxSOCKET_LOST_FLAG);

        const long size = wex::config(_("Buffer Size")).get(4096);

        if (size <= 0)
        {
          std::cerr << "illegal buffer size, skipping socket input data";
          return;
        }

        char* buffer = new char[size];
        sock->Read(buffer, size);

        if (sock->LastReadCount() > 0)
        {
          m_Statistics.inc("Messages Received");
          m_Statistics.inc("Bytes Received", sock->LastReadCount());

          const wxString text(buffer, sock->LastReadCount());

          if (wex::config(_("Log Data")).get(true))
          {
            if (wex::config(_("Count Only")).get(true))
            {
              AppendText(m_LogWindow, 
                wxString::Format(_("read %d bytes from %s"), 
                  sock->LastReadCount(), SocketDetails(sock).c_str()),
                DATA_MESSAGE);
            }
            else
            {
              AppendText(m_LogWindow, text, DATA_READ);
            }
          }
          
          switch (m_Answer)
          {
            case ANSWER_COMMAND: 
              {
              const wxString command(m_Shell->get_command());
              if (command != "history")
              {
                WriteDataToSocket(command.ToAscii(), sock); 
              }
              }
              break;
              
            case ANSWER_ECHO: WriteDataToSocket(text.ToAscii(), sock); break;
            case ANSWER_FILE: WriteDataToSocket(m_DataWindow->GetTextRaw(), sock); break;
          }

          if (manager().GetPane("SHELL").IsShown())
          {
            AppendText(m_Shell, text, DATA_MESSAGE_RAW);
            m_Shell->prompt(std::string(), false); // no eol
          }
        }

        delete [] buffer;

        statustext(
          std::to_string(m_Statistics.get("Bytes Received")) + "," +
          std::to_string(m_Statistics.get("Bytes Sent")),
          "PaneBytes");

#if wxUSE_TASKBARICON
        UpdateTaskBar();
#endif
        // Enable input events again.
        sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG);
        }
        break;

      case wxSOCKET_LOST:
        m_Statistics.inc("Connections Closed");
        
        if (event.GetId() == id_socket_client)
        {
          SocketClosed(sock, true); 
        }
        else
        {
          SocketClosed(sock, false); 
          m_SocketRemoteClient = nullptr;
        }

        UpdateConnectionsPane();

#if wxUSE_TASKBARICON
        UpdateTaskBar();
#endif
        break;

      default:
        assert(0);
   };}, id_socket_remoteclient, id_socket_client);

  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
#if wxUSE_TASKBARICON
    if (event.CanVeto())
    {
      Hide();
      return;
    }
#endif
    if (wex::file_dialog(
      &m_DataWindow->get_file()).show_modal_if_changed() == wxID_CANCEL)
    {
      return;
    }

    if (m_SocketRemoteClient != nullptr)
    {
      m_SocketRemoteClient->Destroy();
      m_SocketRemoteClient = nullptr;
    }

    for (auto c : m_Clients) c->Destroy();
    m_Clients.clear();

    wex::config("Perspective").set(manager().SavePerspective().ToStdString());
    event.Skip();});
    
  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    WriteDataWindowToConnections();});
    
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_SocketServer == nullptr);}, wxID_EXECUTE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_DataWindow->GetModify());}, wxID_SAVE);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_SocketServer != nullptr);}, wxID_STOP);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_SocketRemoteClient == nullptr);}, id_remote_server_connect);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
   event.Enable(m_SocketRemoteClient != nullptr);}, id_remote_server_disconnect);

  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!m_Statistics.get_items().empty());}, id_clear_statistics);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(wex::config(_("Log Data")).get(true));}, id_client_log_data);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(wex::config(_("Log Data")).get(true));
    event.Check(wex::config(_("Count Only")).get(true));}, id_client_log_data_count_only);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(!file_history().get_history_file().data().empty());}, id_recent_file_menu);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Timer.IsRunning());}, id_timer_stop);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("DATA").IsShown());}, id_view_data);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("LOG").IsShown());}, id_view_log);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("SHELL").IsShown());}, id_view_shell);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(manager().GetPane("STATISTICS").IsShown());}, id_view_statistics);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(
      (!m_Clients.empty() || 
        (m_SocketRemoteClient != nullptr && m_SocketRemoteClient->IsConnected())) && 
       m_DataWindow->GetLength() > 0);}, id_write_data);

  statustext(wex::lexers::get()->theme(), "PaneTheme");
}

frame::~frame()
{
#if wxUSE_TASKBARICON
  delete m_TaskBarIcon;
#endif
  delete m_SocketRemoteClient;
  delete m_SocketServer;
}

void frame::AppendText(wex::stc* stc, const wxString& text, int mode)
{
  const bool pos_at_end = (stc->GetCurrentPos() == stc->GetTextLength());
  
  wxString prefix;
  
  switch (mode)
  {
    case DATA_MESSAGE: break;
    case DATA_MESSAGE_RAW: break;
    case DATA_READ: prefix = "r: "; break;
    case DATA_WRITE: prefix = "w: "; break;
  }

  if (mode != DATA_MESSAGE_RAW)
  {
    stc->AppendText(wxDateTime::Now().Format() + " " + prefix);
  }
  
  if (!stc->is_hexmode() || mode == DATA_MESSAGE || mode == DATA_MESSAGE_RAW)
  {
    stc->AppendText(text);
  }
  else
  {
    stc->get_hexmode().append_text(text.ToStdString());
  }

  if (!text.EndsWith("\n"))
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

void frame::LogConnection(
  wxSocketBase* sock,
  bool opend,
  bool show_connections)
{
  wxString text;
  text << (opend ? _("opened"): _("closed")) << " " << SocketDetails(sock);

  const auto connections = m_Clients.size() + 
    (m_SocketRemoteClient != nullptr && m_SocketRemoteClient->IsConnected() ? 1: 0);

  if (show_connections && connections > 0)
  {
    text << " " << _("connections: ") << connections;
  }

  AppendText(m_LogWindow, text, DATA_MESSAGE);

  UpdateConnectionsPane();
}

void frame::on_command_item_dialog(
  wxWindowID dialogid,
  const wxCommandEvent& event)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (event.GetId() != wxID_CANCEL)
    {
      m_DataWindow->config_get();
      m_LogWindow->config_get();
      m_Shell->config_get();
    }
  }
  else
  {
    wex::report::frame::on_command_item_dialog(dialogid, event);
  }
}

wex::stc* frame::open_file(const wex::path& filename, const wex::stc_data& data)
{
  if (m_DataWindow->open(filename, data))
  {
    show_pane("DATA");
  }
  
  return m_DataWindow;
}

bool frame::SetupSocketServer()
{
  if (m_SocketServer != nullptr)
  {
    // In fact, this should not happen, 
    // but using Ubuntu the OnUpdateUI does not prevent this...
    wex::log::status(_("First stop server"));
    return false;
  }
  
  // Create the address - defaults to localhost and port as specified
  if (!wex::config(_("Hostname")).exists())
  {
    wex::config::set_record_defaults(true);
  }

  wxIPV4address addr;
  addr.Hostname(wex::config(_("Hostname")).get("localhost"));
  addr.Service(wex::config(_("Port")).get( 3000));
  wex::config::set_record_defaults(false);

  m_SocketServer = new wxSocketServer(addr);

  wxString text;

  // We use Ok() here to see if the server is really listening
  if (!m_SocketServer->Ok())
  {
    text = wxString::Format(_("could not listen at %ld"), 
      wex::config(_("Port")).get(3000));
      
#if wxUSE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(notready), text);
#endif

    m_SocketServer->Destroy();
    m_SocketServer = nullptr;
    
    wex::log::status(text);
    AppendText(m_LogWindow, text, DATA_MESSAGE);
    
    return false;
  }
  else
  {
    text = wxString::Format(_("server listening at %ld"), 
      wex::config(_("Port")).get(3000));

#if wxUSE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(ready), text);
#endif
  }

  wex::log::status(text);
  AppendText(m_LogWindow, text, DATA_MESSAGE);

  // Setup the event handler and subscribe to connection events
  m_SocketServer->SetEventHandler(*this, id_socket_server);
  m_SocketServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
  m_SocketServer->Notify(true);

  return true;
}

void frame::SocketClosed(wxSocketBase* sock, bool remove_from_clients)
{
  assert(sock != nullptr);

  if (remove_from_clients)
  {
    m_Clients.remove(sock);
  }

  LogConnection(sock, false, remove_from_clients);

  sock->Destroy();
}

const wxString frame::SocketDetails(const wxSocketBase* sock) const
{
  assert(sock != nullptr);

  wxIPV4address peer_addr;

  if (!sock->GetPeer(peer_addr))
  {
    std::cerr << "could not get peer address";
    return wxEmptyString;
  }

  wxIPV4address local_addr;

  if (!sock->GetLocal(local_addr))
  {
    std::cerr << "could not get local address";
    return wxEmptyString;
  }

  wxString value;

  value <<
    _("socket: ") <<
    local_addr.IPAddress() << "." << (int)local_addr.Service() << ", " <<
    peer_addr.IPAddress() << "." << (int)peer_addr.Service();

  return value;
}

void frame::statusbar_clicked(const std::string& pane)
{
  if (pane == "PaneTimer")
  {
    TimerDialog();
  }
  else if (pane == "PaneTheme")
  {
    if (wex::lexers::get()->show_theme_dialog(this))
    {
      m_DataWindow->get_lexer().set(m_DataWindow->get_lexer().display_lexer());
      m_LogWindow->get_lexer().set(m_LogWindow->get_lexer().display_lexer());
      m_Shell->get_lexer().set(m_Shell->get_lexer().display_lexer());

      m_StatusBar->show_field(
        "PaneLexer", 
        !wex::lexers::get()->theme().empty());
        
      statustext(wex::lexers::get()->theme(), "PaneTheme");
    }
  }
  else
  {
    wex::report::frame::statusbar_clicked(pane);
  }
}

void frame::TimerDialog()
{
  const long val = wxGetNumberFromUser(
    _("Input (seconds):"),
    wxEmptyString,
    _("Repeat Timer"),
    wex::config(_("Timer")).get(60),
    1,
    3600 * 24);

  // If cancelled, -1 is returned.
  if (val == -1) return;

  wex::config(_("Timer")).set(val);

  if (val > 0)
  {
    m_Timer.Start(1000 * val);
    
    AppendText(m_LogWindow,
      wxString::Format(_("timer set to: %ld seconds (%s)"),
      val,
      wxTimeSpan(0, 0, val, 0).Format().c_str()),
      DATA_MESSAGE);
      
    statustext(std::to_string(val), "PaneTimer");
  }
  else if (val == 0)
  {
    m_Timer.Stop();
    
    AppendText(m_LogWindow, _("timer stopped"), DATA_MESSAGE);
    
    statustext(std::string(), "PaneTimer");
  }
}

void frame::UpdateConnectionsPane() const
{
  statustext(
    std::to_string(m_Clients.size()) + "," + 
    std::to_string(m_SocketRemoteClient != nullptr && m_SocketRemoteClient->IsConnected() ? 1: 0), 
    "PaneConnections");
}

#if wxUSE_TASKBARICON
void frame::UpdateTaskBar()
{
  if (m_Clients.empty())
  {
    m_TaskBarIcon->SetIcon(
      wxICON(ready), 
      wxString::Format(_("server listening at %ld"), 
        wex::config(_("Port")).get(3000)));
  }
  else
  {
    const wxString text =
      wxString::Format(
        _("%s %ld connections connected at %ld\nreceived: %d bytes sent: %d bytes"),
        wxTheApp->GetAppName().c_str(),
        m_Clients.size() + 
          (m_SocketRemoteClient != nullptr && m_SocketRemoteClient->IsConnected() ? 1: 0),
        wex::config(_("Port")).get(3000),
        m_Statistics.get("Bytes Received"),
        m_Statistics.get("Bytes Sent"));

    m_TaskBarIcon->SetIcon(wxICON(connect), text);
  }
}
#endif

void frame::WriteDataToSocket(const wxCharBuffer& buffer, wxSocketBase* sock)
{
  if (buffer.length() == 0) return;

  size_t written = 0;

  while (written < buffer.length())
  {
    if (sock->Write(buffer.data() + written, buffer.length() - written).Error())
    {
      wex::log::status("Socket Error") << std::to_string(sock->LastError());
      m_Statistics.inc("Socket Error");
    }

    written += sock->LastWriteCount();
  }

  m_Statistics.inc("Bytes Sent", written);
  m_Statistics.inc("Messages Sent");

  statustext(
    std::to_string(m_Statistics.get("Bytes Received")) + "," + 
    std::to_string(m_Statistics.get("Bytes Sent")),
    "PaneBytes");

#if wxUSE_TASKBARICON
  UpdateTaskBar();
#endif

  if (wex::config(_("Log Data")).get(true))
  {
    if (wex::config(_("Count Only")).get(true))
    {
      AppendText(m_LogWindow,
        wxString::Format(_("write %d bytes to %s"),
          written, SocketDetails(sock).c_str()),
        DATA_MESSAGE);
    }
    else
    {
      AppendText(m_LogWindow, buffer, DATA_WRITE);
    }
  }
}

void frame::WriteDataWindowToConnections()
{
  const wxCharBuffer& buffer = m_DataWindow->GetTextRaw();

  for (auto& it : m_Clients)
  {
    WriteDataToSocket(buffer, it);
  }

  if (m_SocketRemoteClient != nullptr && m_SocketRemoteClient->IsConnected())
  {
    WriteDataToSocket(buffer, m_SocketRemoteClient);
  }
}

const auto id_open = wxWindow::NewControlId();

#if wxUSE_TASKBARICON
taskbar_icon::taskbar_icon(frame* frame)
  : m_Frame(frame) 
{
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Frame->Close(true);}, wxID_EXIT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_Frame->Show();}, ID_OPEN);

  Bind(wxEVT_TASKBAR_LEFT_DCLICK, [=](wxTaskBarIconEvent&) {
    m_Frame->Show();});
    
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Enable(m_Frame->server_not_listening());}, wxID_EXIT);
}

wxMenu *taskbar_icon::CreatePopupMenu()
{
  wex::menu* menu = new wex::menu;
  menu->Append(ID_OPEN, _("Open"));
  menu->append_separator();
  menu->Append(wxID_EXIT);

  return menu;
}
#endif
#endif
