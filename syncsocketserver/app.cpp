////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <functional>
#include <algorithm>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/textfile.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/grid.h>
#include <wx/extension/statistics.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include "app.h"

#ifndef __WXMSW__
#include "app.xpm"
#include "connect.xpm"
#include "notready.xpm"
#include "ready.xpm"
#endif

#if wxUSE_SOCKETS

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
  SetAppName("syncsocketserver");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  // Show (and possibly Hide) is in the constuctor.
  new Frame();

  return true;
}

BEGIN_EVENT_TABLE(Frame, wxExFrameWithHistory)
  EVT_CLOSE(Frame::OnClose)
  EVT_MENU(wxID_EXECUTE, Frame::OnCommand)
  EVT_MENU(wxID_STOP, Frame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, Frame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_CLOSE_ALL, Frame::OnCommand)
  EVT_MENU_RANGE(ID_MENU_FIRST, ID_MENU_LAST, Frame::OnCommand)
  EVT_SOCKET(ID_SERVER, Frame::OnSocket)
  EVT_SOCKET(ID_CLIENT, Frame::OnSocket)
  EVT_TIMER(-1, Frame::OnTimer)
  EVT_UPDATE_UI(wxID_EXECUTE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVE, Frame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLEAR_STATISTICS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLIENT_ECHO, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLIENT_LOG_DATA, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLIENT_LOG_DATA_COUNT_ONLY, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SERVER_CONFIG, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_TIMER_STOP, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_DATA, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_LOG, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_SHELL, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATISTICS, Frame::OnUpdateUI)
  EVT_UPDATE_UI(ID_WRITE_DATA, Frame::OnUpdateUI)
END_EVENT_TABLE()

Frame::Frame()
  : wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName())
  , m_Timer(this)
{
  SetIcon(wxICON(app));

#if wxUSE_TASKBARICON
  m_TaskBarIcon = new TaskBarIcon(this);
#endif

  Show(); // otherwise statusbar is not placed correctly

#if wxUSE_STATUSBAR
  // Statusbar setup before STC construction.
  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("PaneClients", 75, _("Number of clients connected")));
  panes.push_back(wxExStatusBarPane("PaneTimer", 75, _("Repeat timer")));
  panes.push_back(wxExStatusBarPane("PaneBytes", 150, _("Number of bytes received and sent")));
  panes.push_back(wxExStatusBarPane("PaneFileType", 50, _("File type")));
  panes.push_back(wxExStatusBarPane("PaneInfo", 100, _("Lines")));
  SetupStatusBar(panes);
#endif

  m_DataWindow = new wxExSTCWithFrame(this, this);

  m_LogWindow = new wxExSTC(this, wxEmptyString, 0, _("Log"));

  m_Shell = new wxExSTCShell(this);

  m_LogWindow->ResetMargins();

  wxExMenu* menuFile = new wxExMenu();
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENT_FILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(ID_CLEAR_STATISTICS, 
    _("Clear Statistics"), _("Clears the statistics"));
  menuFile->AppendSeparator();
#if wxUSE_TASKBARICON
  menuFile->Append(ID_HIDE, _("Hide"), _("Puts back in the task bar"));
#else
  menuFile->Append(wxID_EXIT);
#endif

  wxMenu* menuServer = new wxMenu();
  menuServer->Append(ID_SERVER_CONFIG,
    wxExEllipsed(_("Config")), _("Configures the server"));
  menuServer->AppendSeparator();
  menuServer->Append(wxID_EXECUTE);
  menuServer->Append(wxID_STOP);

  wxExMenu* menuClient = new wxExMenu();
  menuClient->AppendCheckItem(ID_CLIENT_ECHO, _("Echo"),
    _("Echo's received data back to client"));
  menuClient->AppendCheckItem(ID_CLIENT_LOG_DATA, _("Log Data"),
    _("Logs data read from and written to client"));
  menuClient->AppendCheckItem(ID_CLIENT_LOG_DATA_COUNT_ONLY, _("Count Only"),
    _("Logs only byte counts, no text"));
  menuClient->AppendSeparator();
  menuClient->Append(ID_CLIENT_BUFFER_SIZE, wxExEllipsed(_("Buffer Size")),
    _("Sets buffersize for data retrieved from client"));
  menuClient->AppendSeparator();
  menuClient->Append(ID_TIMER_START, wxExEllipsed(_("Repeat Timer")),
    _("Repeats with timer writing last data to all clients"));
  menuClient->Append(ID_TIMER_STOP, _("Stop Timer"), _("Stops the timer"));
  menuClient->AppendSeparator();
  menuClient->Append(ID_WRITE_DATA, _("Write"),
    _("Writes data to all clients"), wxART_GO_FORWARD);

  wxExMenu* menuView = new wxExMenu();
  menuView->AppendBars();
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_LOG, _("Log"));
  menuView->AppendCheckItem(ID_VIEW_DATA, _("Data"));
  menuView->AppendCheckItem(ID_VIEW_SHELL, _("Shell"));
  menuView->AppendCheckItem(ID_VIEW_STATISTICS, _("Statistics"));

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(wxID_PREFERENCES);

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menuBar->Append(menuView, _("&View"));
  menuBar->Append(menuServer, _("&Server"));
  menuBar->Append(menuClient, _("&Client"));
  menuBar->Append(menuOptions, _("&Options"));
  menuBar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  SetMenuBar(menuBar);

  GetManager().AddPane(m_LogWindow,
    wxAuiPaneInfo().CenterPane().Name("LOG"));
  GetManager().AddPane(m_DataWindow,
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Data")).Name("DATA"));
  GetManager().AddPane(m_Shell,
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Shell")).Name("SHELL"));
  GetManager().AddPane(m_Statistics.Show(this),
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).Caption(_("Statistics")).Name("STATISTICS"));
  GetManager().LoadPerspective(wxConfigBase::Get()->Read("Perspective"));

  if (SetupSocketServer())
  {
#if wxUSE_TASKBARICON
    Hide();
#endif
  }

  if (!GetRecentFile().empty() && GetManager().GetPane("DATA").IsShown())
  {
    OpenFile(GetRecentFile());
  }

  if (GetManager().GetPane("SHELL").IsShown())
  {
    m_Shell->SetFocus();
    m_Shell->DocumentEnd();
  }

  GetToolBar()->AddTool(
    ID_WRITE_DATA,
    wxEmptyString,
    wxArtProvider::GetBitmap(
      wxART_GO_FORWARD, wxART_TOOLBAR, GetToolBar()->GetToolBitmapSize()),
    _("Write data"));
  GetToolBar()->Realize();
    
  GetManager().Update();
}

Frame::~Frame()
{
#if wxUSE_TASKBARICON
  delete m_TaskBarIcon;
#endif
  delete m_SocketServer;
}

void Frame::AppendText(wxExSTC* stc, const wxString& text, bool withTimestamp)
{
  const bool pos_at_end = (stc->GetCurrentPos() == stc->GetTextLength());
  
  bool readonly = false;
  
  if (stc->GetReadOnly())
  {
    readonly = true;
    stc->SetReadOnly(false);
  }
  
  const bool hex = (stc->GetFlags() & wxExSTC::STC_WIN_HEX);

  if (withTimestamp && !hex)
  {
    const wxString now = wxDateTime::Now().Format();
    stc->AppendText(now + " " + text + stc->GetEOL());
  }
  else
  {
    if (hex)
    {
      stc->AddTextHexMode(stc->GetLength(), text.c_str());
    }
    else
    {
      // No GetEOL, that is only added with timestamps.
      stc->AppendText(text);
    }
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();

  if (pos_at_end)
  {
    stc->DocumentEnd();
  }
  
  if (readonly)
  {
    stc->SetReadOnly(true);
  }
}

void Frame::LogConnection(
  wxSocketBase* sock,
  bool accepted,
  bool show_clients)
{
  wxString text;

  if (sock != NULL)
  {
    text << (accepted ? _("accepted"): _("lost")) << " " << SocketDetails(sock);
  }
  else
  {
    text << (accepted ? _("accepted"): _("lost")) << " " << _("connection");
  }

  if (show_clients)
  {
    text << " " << _("clients: ") << m_Clients.size();
  }

  AppendText(m_LogWindow, text);
}

void Frame::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    Hide();
    return;
  }

  wxExFileDialog dlg(this, &m_DataWindow->GetFile());

  if (dlg.ShowModalIfChanged() == wxID_CANCEL)
  {
    return;
  }

  for_each (m_Clients.begin(), m_Clients.end(), 
    std::mem_fun(&wxSocketBase::Destroy));

  m_Clients.clear();

  wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());
  event.Skip();
}

void Frame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general socket server."));
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
    wxAboutBox(info);
    }
    break;

  case wxID_EXECUTE:
    SetupSocketServer();
    break;

  case wxID_EXIT:
    Close(true);
    break;

  case wxID_NEW:
    m_DataWindow->GetFile().FileNew(wxExFileName());
    GetManager().GetPane("DATA").Show();
    GetManager().Update();
    break;

  case wxID_OPEN:
    wxExOpenFilesDialog(
      this, 
      wxFD_OPEN | wxFD_CHANGE_DIR, 
      wxFileSelectorDefaultWildcardStr, 
      true);
    break;

  case wxID_SAVE:
    m_DataWindow->GetFile().FileSave();
    break;

  case wxID_SAVEAS:
    {
      wxExFileDialog dlg(
        this, 
        &m_DataWindow->GetFile(), 
        wxGetStockLabel(wxID_SAVEAS), 
        wxFileSelectorDefaultWildcardStr, 
        wxFD_SAVE);
        
      if (dlg.ShowModal())
      {
        m_DataWindow->GetFile().FileSave(dlg.GetPath());
      }
    }
    break;

  case wxID_STOP:
    {
      if (wxMessageBox(_("Stop server?"),
        _("Confirm"),
        wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
      {
        return;
      }

      for (
        auto it = m_Clients.begin();
        it != m_Clients.end();
        ++it)
      {
        wxSocketBase* sock = *it;
        SocketLost(sock, false);
      }

      m_Clients.clear();

      m_SocketServer->Destroy();
      m_SocketServer = NULL;

      const wxString text = _("server stopped");

#if wxUSE_TASKBARICON
      m_TaskBarIcon->SetIcon(wxICON(notready), text);
#endif
#if wxUSE_STATUSBAR
      StatusText(
        wxString::Format(_("%d clients"), m_Clients.size()),
        "PaneClients");
#endif

      wxLogStatus(text);
      AppendText(m_LogWindow, text);

      const wxString statistics = m_Statistics.Get();

      if (!statistics.empty())
      {
        AppendText(m_LogWindow, statistics);
      }
    }
    break;

  case ID_CLEAR_STATISTICS:
    m_Statistics.Clear();
    break;

  case ID_CLIENT_BUFFER_SIZE:
    {
    long val;
    if ((val = wxGetNumberFromUser(
      _("Input") + ":",
      wxEmptyString,
      _("Buffer Size"),
      wxConfigBase::Get()->ReadLong(_("Buffer Size"), 4096),
      1,
      65536)) > 0)
      {
        wxConfigBase::Get()->Write(_("Buffer Size"), val);
      }
    }
    break;

  case ID_CLIENT_ECHO:
    wxConfigBase::Get()->Write(_("Echo"), 
      !wxConfigBase::Get()->ReadBool(_("Echo"), false));
    break;

  case ID_CLIENT_LOG_DATA:
    wxConfigBase::Get()->Write(_("Log Data"), 
      !wxConfigBase::Get()->ReadBool(_("Log Data"), false));
    break;

  case ID_CLIENT_LOG_DATA_COUNT_ONLY:
    wxConfigBase::Get()->Write(_("Count Only"), 
      !wxConfigBase::Get()->ReadBool(_("Count Only"), true));
    break;

  case ID_HIDE:
    Close(false);
    break;

  case ID_SERVER_CONFIG:
    {
    std::vector<wxExConfigItem> v;
    v.push_back(wxExConfigItem(_("Hostname"), 
      wxEmptyString, 
      wxEmptyString,
      0, 
      CONFIG_STRING,
      true));
      
    // Well known ports are in the range from 0 to 1023.
    // Just allow here for most flexibility.
    v.push_back(wxExConfigItem(_("Port"), 1, 65536));

    // Configuring only possible if server is stopped,
    // otherwise just show settings readonly mode.
    const long flags = (m_SocketServer == NULL ? wxOK|wxCANCEL: wxCANCEL);

    wxExConfigDialog(this,
      v,
      _("Server Config"),
      0,
      1,
      flags).ShowModal();
    }
    break;

  case ID_SHELL_COMMAND:
    {
      const wxString str = event.GetString() + wxTextFile::GetEOL();
      const wxCharBuffer& buffer(str.c_str());

      for (
        auto it = m_Clients.begin();
        it != m_Clients.end();
        ++it)
      {
        wxSocketBase* sock = *it;
        WriteDataToClient(buffer, sock);
      }

      m_Shell->Prompt();
    }
    break;

  case ID_TIMER_START: TimerDialog(); break;

  case ID_TIMER_STOP:
    m_Timer.Stop();
    AppendText(m_LogWindow, _("timer stopped"));
#if wxUSE_STATUSBAR
    StatusText(wxEmptyString, "PaneTimer");
#endif
    break;

  case ID_VIEW_DATA: TogglePane("DATA"); break;
  case ID_VIEW_LOG: TogglePane("LOG"); break;
  case ID_VIEW_SHELL: TogglePane("SHELL"); break;
  case ID_VIEW_STATISTICS: TogglePane("STATISTICS"); break;

  case ID_WRITE_DATA:
    WriteDataWindowToClients();
    break;

  default:
    wxFAIL;
  }
}

void Frame::OnCommandConfigDialog(
  wxWindowID dialogid,
  int commandid)
{
  if (dialogid == wxID_PREFERENCES)
  {
    if (commandid != wxID_CANCEL)
    {
      m_DataWindow->ConfigGet();
      m_LogWindow->ConfigGet();
    }
  }
  else
  {
    wxExFrameWithHistory::OnCommandConfigDialog(dialogid, commandid);
  }
}

void Frame::OnSocket(wxSocketEvent& event)
{
  wxSocketBase *sock = event.GetSocket();

  if (event.GetId() == ID_SERVER)
  {
    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).
    sock = m_SocketServer->Accept(false);

    if (sock == NULL)
    {
      AppendText(m_LogWindow,
        _("error: couldn't accept a new connection"));
      return;
    }

    m_Statistics.Inc(_("Connections Accepted"));

    sock->SetEventHandler(*this, ID_CLIENT);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    m_Clients.push_back(sock);

#if wxUSE_STATUSBAR
    StatusText(
      wxString::Format(_("%d clients"), m_Clients.size()),
      "PaneClients");
#endif

    LogConnection(sock, true);

    const wxCharBuffer& buffer = m_DataWindow->GetTextRaw();
    WriteDataToClient(buffer, sock);

#if wxUSE_TASKBARICON
    UpdateTaskBar();
#endif
  }
  else if (event.GetId() == ID_CLIENT)
  {
    switch (event.GetSocketEvent())
    {
      case wxSOCKET_INPUT:
      {
        m_Statistics.Inc(_("Input Events"));

        // We disable input events, so that the test doesn't trigger
        // wxSocketEvent again.
        sock->SetNotify(wxSOCKET_LOST_FLAG);

        const auto size = wxConfigBase::Get()->ReadLong(_("Buffer Size"), 4096);

        if (size <= 0)
        {
          wxLogError("Illegal buffer size, skipping socket input data");
          return;
        }

        char* buffer = new char[size];
        sock->Read(buffer, size);

        m_Statistics.Inc(_("Bytes Received"), sock->LastCount());

        if (sock->LastCount() > 0)
        {
          if (wxConfigBase::Get()->ReadBool(_("Echo"), false))
          {
            sock->Write(buffer, sock->LastCount());
            SocketCheckError(sock);
            m_Statistics.Inc(_("Bytes Sent"), sock->LastCount());
          }

          const wxString text(buffer, sock->LastCount());

          if (GetManager().GetPane("SHELL").IsShown())
          {
            AppendText(m_Shell, text, false);

            if (text.EndsWith("\n"))
            {
              m_Shell->Prompt(wxEmptyString, false); // no eol
            }
          }

          if (wxConfigBase::Get()->ReadBool(_("Log Data"), true))
          {
            if (wxConfigBase::Get()->ReadBool(_("Count Only"), true))
            {
              AppendText(m_LogWindow, 
                wxString::Format(_("read: %d bytes from: %s"), 
                  sock->LastCount(), SocketDetails(sock).c_str()));
            }
            else
            {
              AppendText(m_LogWindow, text);
            }
          }
        }

        delete [] buffer;

#if wxUSE_STATUSBAR
        StatusText(wxString::Format("%d,%d",
          m_Statistics.Get(_("Bytes Received")),
          m_Statistics.Get(_("Bytes Sent"))),
          "PaneBytes");
#endif

#if wxUSE_TASKBARICON
        UpdateTaskBar();
#endif
        // Enable input events again.
        sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
        break;
      }

      case wxSOCKET_LOST:
        m_Statistics.Inc(_("Connections Lost"));
        SocketLost(sock, true);
#if wxUSE_STATUSBAR
        StatusText(
          wxString::Format(_("%d clients"), m_Clients.size()),
          "PaneClients");
#endif

#if wxUSE_TASKBARICON
        UpdateTaskBar();
#endif
        break;

      default:
        wxFAIL;
    }
  }
  else
  {
    wxFAIL;
  }
}

void Frame::OnTimer(wxTimerEvent& /* event */)
{
  WriteDataWindowToClients();
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case wxID_SAVE:
    event.Enable(m_DataWindow->GetModify());
    break;

  case wxID_EXECUTE:
    event.Enable(m_SocketServer == NULL);
    break;

  case wxID_STOP:
    event.Enable(m_SocketServer != NULL);
    break;

  case ID_CLIENT_ECHO:
    event.Check(wxConfigBase::Get()->ReadBool(_("Echo"), false));
    break;

  case ID_CLIENT_LOG_DATA:
    event.Check(wxConfigBase::Get()->ReadBool(_("Log Data"), false));
    break;

  case ID_CLIENT_LOG_DATA_COUNT_ONLY:
    event.Enable(wxConfigBase::Get()->ReadBool(_("Log Data"), false));
    event.Check(wxConfigBase::Get()->ReadBool(_("Count Only"), true));
    break;

  case ID_CLEAR_STATISTICS:
    event.Enable(!m_Statistics.GetItems().empty());
    break;

  case ID_RECENT_FILE_MENU:
    event.Enable(!GetRecentFile().empty());
    break;

  case ID_SERVER_CONFIG:
    break;

  case ID_TIMER_STOP:
    event.Enable(m_Timer.IsRunning());
    break;

  case ID_VIEW_DATA:
    event.Check(GetManager().GetPane("DATA").IsShown());
    break;

  case ID_VIEW_LOG:
    event.Check(GetManager().GetPane("LOG").IsShown());
    break;

  case ID_VIEW_SHELL:
    event.Check(GetManager().GetPane("SHELL").IsShown());
    break;

  case ID_VIEW_STATISTICS:
    event.Check(GetManager().GetPane("STATISTICS").IsShown());
    break;

  case ID_WRITE_DATA:
    event.Enable(m_Clients.size() > 0 && m_DataWindow->GetLength() > 0);
    break;

  default:
    wxFAIL;
  }
}

bool Frame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  if (m_DataWindow->Open(filename, line_number, match, flags))
  {
    GetManager().GetPane("DATA").Show();
    GetManager().Update();

    AppendText(m_LogWindow,
      _("opened: ") + filename.GetFullPath() + wxString::Format(" (%d bytes)",
      m_DataWindow->GetLength()));

    return true;
  }
  else
  {
    return false;
  }
}

bool Frame::SetupSocketServer()
{
  // Create the address - defaults to localhost and port as specified
  if (!wxConfigBase::Get()->Exists(_("Hostname")))
  {
    wxConfigBase::Get()->SetRecordDefaults(true);
  }

  wxIPV4address addr;
  addr.Hostname(wxConfigBase::Get()->Read(_("Hostname"), "localhost"));
  addr.Service(wxConfigBase::Get()->ReadLong(_("Port"), 3000));

  if (wxConfigBase::Get()->IsRecordingDefaults())
  {
    wxConfigBase::Get()->SetRecordDefaults(false);
  }

  // Create the socket
  m_SocketServer = new wxSocketServer(addr);

  wxString text;

  // We use Ok() here to see if the server is really listening
  if (!m_SocketServer->Ok())
  {
    text = wxString::Format(_("could not listen at %d"), 
      wxConfigBase::Get()->ReadLong(_("Port"), 3000));
      
#if wxUSE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(notready), text);
#endif

    m_SocketServer->Destroy();
    m_SocketServer = NULL;
    
    wxLogStatus(text);
    AppendText(m_LogWindow, text);
    
    return false;
  }
  else
  {
    text = wxString::Format(_("server listening at %d"), 
        wxConfigBase::Get()->ReadLong(_("Port"), 3000));

#if wxUSE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(ready), text);
#endif
  }

  wxLogStatus(text);
  AppendText(m_LogWindow, text);

  // Setup the event handler and subscribe to connection events
  m_SocketServer->SetEventHandler(*this, ID_SERVER);
  m_SocketServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
  m_SocketServer->Notify(true);

  return true;
}

bool Frame::SocketCheckError(const wxSocketBase* sock)
{
  if (sock->Error())
  {
    const wxString error = 
      wxString::Format(_("Socket Error: %d"), sock->LastError());
      
    wxLogStatus(error);

    m_Statistics.Inc(error);
    
    return true;
  }

  return false;
}

const wxString Frame::SocketDetails(const wxSocketBase* sock) const
{
  // See invocation at SocketLost.
  // If that is solved we can wxASSERT(sock != NULL) again.
  if (sock == NULL)
  {
    return wxEmptyString;
  }

  wxIPV4address peer_addr;

  if (!sock->GetPeer(peer_addr))
  {
    wxLogError("Could not get peer address");
    return wxEmptyString;
  }

  wxIPV4address local_addr;

  if (!sock->GetLocal(local_addr))
  {
    wxLogError("Could not get local address");
    return wxEmptyString;
  }

  wxString value;

  value <<
    _("socket: ") <<
    local_addr.IPAddress() << "." << (int)local_addr.Service() << ", " <<
    peer_addr.IPAddress() << "." << (int)peer_addr.Service();

  return value;
}

void Frame::SocketLost(wxSocketBase* sock, bool remove_from_clients)
{
  wxASSERT(sock != NULL);

  if (remove_from_clients)
  {
    m_Clients.remove(sock);
  }

#ifdef __WXMSW__
  LogConnection(sock, false, remove_from_clients);
#else
  LogConnection(NULL, false, remove_from_clients); // otherwise a crash!
#endif

  sock->Destroy();
}

void Frame::StatusBarDoubleClicked(const wxString& pane)
{
  if (pane == "PaneTimer")
  {
    TimerDialog();
  }
  else
  {
    wxExFrameWithHistory::StatusBarDoubleClicked(pane);
  }
}

void Frame::TimerDialog()
{
  const auto val = wxGetNumberFromUser(
    _("Input (seconds):"),
    wxEmptyString,
    _("Repeat Timer"),
    wxConfigBase::Get()->ReadLong(_("Timer"), 60),
    1,
    3600 * 24);

  // If cancelled, -1 is returned.
  if (val == -1) return;

  wxConfigBase::Get()->Write(_("Timer"), val);

  if (val > 0)
  {
    m_Timer.Start(1000 * val);
    
    AppendText(m_LogWindow,
      wxString::Format(_("timer set to: %d seconds (%s)"),
      val,
      wxTimeSpan(0, 0, val, 0).Format().c_str()));
      
#if wxUSE_STATUSBAR
    StatusText(wxString::Format("%ld", val), "PaneTimer");
#endif
  }
  else if (val == 0)
  {
    m_Timer.Stop();
    
    AppendText(m_LogWindow, _("timer stopped"));
    
#if wxUSE_STATUSBAR
    StatusText(wxEmptyString, "PaneTimer");
#endif
  }
}

#if wxUSE_TASKBARICON
void Frame::UpdateTaskBar()
{
  if (m_Clients.size() == 0)
  {
    m_TaskBarIcon->SetIcon(
      wxICON(ready), 
      wxString::Format(_("server listening at %d"), 
        wxConfigBase::Get()->ReadLong(_("Port"), 3000)));
  }
  else
  {
    const wxString text =
      wxString::Format(
        _("%s %d clients connected at %d\nreceived: %ld bytes sent: %ld bytes"),
          wxTheApp->GetAppName().c_str(),
          m_Clients.size(),
          wxConfigBase::Get()->ReadLong(_("Port"), 3000),
          m_Statistics.Get(_("Bytes Received")),
          m_Statistics.Get(_("Bytes Sent")));

    m_TaskBarIcon->SetIcon(wxICON(connect), text);
  }
}
#endif

void Frame::WriteDataToClient(const wxCharBuffer& buffer, wxSocketBase* client)
{
  if (buffer.length() == 0) return;

  client->Write(buffer, buffer.length());

  if (SocketCheckError(client))
  {
    return;
  }

  if (client->LastCount() != buffer.length())
  {
    AppendText(m_LogWindow, _("not all bytes sent to socket"));
  }

  m_Statistics.Inc(_("Bytes Sent"), client->LastCount());
  m_Statistics.Inc(_("Messages Sent"));

#if wxUSE_STATUSBAR
  StatusText(wxString::Format("%d,%d",
    m_Statistics.Get(_("Bytes Received")), m_Statistics.Get(_("Bytes Sent"))),
    "PaneBytes");
#endif

#if wxUSE_TASKBARICON
  UpdateTaskBar();
#endif

  if (wxConfigBase::Get()->ReadBool(_("Log Data"), true))
  {
    if (wxConfigBase::Get()->ReadBool(_("Count Only"), true))
    {
      AppendText(m_LogWindow,
        wxString::Format(_("write: %d bytes to: %s"),
          client->LastCount(), SocketDetails(client).c_str()));
    }
    else
    {
      AppendText(m_LogWindow, buffer);
    }
  }
}

void Frame::WriteDataWindowToClients()
{
  const wxCharBuffer& buffer = m_DataWindow->GetTextRaw();

  for (
    auto it = m_Clients.begin();
    it != m_Clients.end();
    ++it)
  {
    wxSocketBase* sock = *it;
    WriteDataToClient(buffer, sock);
  }
}

enum
{
  ID_OPEN = ID_CLIENT + 1,
};

#if wxUSE_TASKBARICON
BEGIN_EVENT_TABLE(TaskBarIcon, wxTaskBarIcon)
  EVT_MENU(wxID_EXIT, TaskBarIcon::OnCommand)
  EVT_MENU(ID_OPEN, TaskBarIcon::OnCommand)
  EVT_TASKBAR_LEFT_DCLICK(TaskBarIcon::OnTaskBarIcon)
  EVT_UPDATE_UI(wxID_EXIT, TaskBarIcon::OnUpdateUI)
END_EVENT_TABLE()

wxMenu *TaskBarIcon::CreatePopupMenu()
{
  wxExMenu* menu = new wxExMenu;
  menu->Append(ID_OPEN, _("Open"));
  menu->AppendSeparator();
  menu->Append(wxID_EXIT);
  return menu;
}

void TaskBarIcon::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_EXIT:
    m_Frame->Close(true);
    break;
  case ID_OPEN:
    m_Frame->Show();
    break;
  default:
    wxFAIL;
    break;
  }
}
#endif // wxUSE_TASKBARICON
#endif // wxUSE_SOCKETS
