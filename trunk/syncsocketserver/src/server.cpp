/******************************************************************************\
* File:          server.cpp
* Purpose:       General socket server
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2007-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/numdlg.h>
#include "server.h"

#ifndef __WXMSW__
#include "appl.xpm"
#include "connect.xpm"
#include "notready.xpm"
#include "ready.xpm"
#endif

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
  SetAppName("syncsocketserver");
  exApp::OnInit();
  MyFrame *frame = new MyFrame("syncsocketserver");
  SetTopWindow(frame);
  return true;
}

BEGIN_EVENT_TABLE(MyFrame, ftFrame)
  EVT_CLOSE(MyFrame::OnClose)
  EVT_MENU(wxID_ABOUT, MyFrame::OnCommand)
  EVT_MENU(wxID_EXIT, MyFrame::OnCommand)
  EVT_MENU(wxID_NEW, MyFrame::OnCommand)
  EVT_MENU(wxID_OPEN, MyFrame::OnCommand)
  EVT_MENU(wxID_SAVE, MyFrame::OnCommand)
  EVT_MENU(wxID_SAVEAS, MyFrame::OnCommand)
  EVT_MENU_RANGE(ID_MENU_FIRST, ID_MENU_LAST, MyFrame::OnCommand)
  EVT_SOCKET(ID_SERVER, MyFrame::OnSocket)
  EVT_SOCKET(ID_CLIENT, MyFrame::OnSocket)
  EVT_TIMER(-1, MyFrame::OnTimer)
  EVT_UPDATE_UI(wxID_SAVE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLEAR_STATISTICS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLIENT_ECHO, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_CLIENT_LOG_DATA, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENT_FILE_MENU, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SERVER_START, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SERVER_STOP, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_SERVER_CONFIG, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_TIMER_STOP, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_DATA, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_LOG, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATISTICS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATUSBAR, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_TOOLBAR, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_WRITE_DATA, MyFrame::OnUpdateUI)
END_EVENT_TABLE()

MyFrame::MyFrame(const wxString& title)
  : ftFrame(NULL, wxID_ANY, title)
  , m_Timer(this)
{
  SetIcon(wxICON(appl));

#ifdef USE_TASKBARICON
  m_TaskBarIcon = new MyTaskBarIcon(this);
#endif

  Show(); // otherwise statusbar is not placed correctly

  // Statusbar setup before STC construction.
  std::vector<exPane> panes;
  panes.push_back(exPane("PaneText", -3));
  panes.push_back(exPane("PaneClients", 75, _("Number of clients connected")));
  panes.push_back(exPane("PaneTimer", 75, _("Repeat timer")));
  panes.push_back(exPane("PaneBytes", 150, _("Number of bytes received and sent")));
  panes.push_back(exPane("PaneFileType", 50, _("File type")));
  panes.push_back(exPane("PaneLines", 100, _("Lines in window")));
  SetupStatusBar(panes);

  m_LogWindow = new ftSTC(this, ftSTC::STC_MENU_SIMPLE | ftSTC::STC_MENU_FIND);
  m_DataWindow = new ftSTC(
    this,
    ftSTC::STC_MENU_SIMPLE | ftSTC::STC_MENU_FIND |
    ftSTC::STC_MENU_REPLACE | ftSTC::STC_MENU_INSERT);

  m_LogWindow->SetReadOnly(true);
  m_LogWindow->SetLexer();
  m_LogWindow->ResetMargins();

  exMenu* menuFile = new exMenu();
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENT_FILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(ID_CLEAR_STATISTICS, _("Clear Statistics"), _("Clears the statistics"));
  menuFile->AppendSeparator();

#ifdef USE_TASKBARICON
  menuFile->Append(ID_HIDE, _("Hide"), _("Puts back in the task bar"));
#else
  menuFile->Append(wxID_EXIT);
#endif

  wxMenu* menuServer = new wxMenu();
  menuServer->Append(ID_SERVER_CONFIG, exEllipsed(_("Config")), _("Configures the server"));
  menuServer->AppendSeparator();
  menuServer->Append(ID_SERVER_START, _("Start"), _("Starts the server"));
  menuServer->Append(ID_SERVER_STOP, exEllipsed(_("Stop")),
    _("Closes connection with all clients and stops the server"));

  exMenu* menuClient = new exMenu();
  menuClient->Append(ID_CLIENT_ECHO, _("Echo"), 
    _("Echo's received data back to client"), wxITEM_CHECK);
  menuClient->Append(ID_CLIENT_LOG_DATA, _("Log Data"), 
    _("Logs data read from and written to client"), wxITEM_CHECK);
  menuClient->AppendSeparator();
  menuClient->Append(ID_CLIENT_BUFFER_SIZE, exEllipsed(_("Set Buffer Size")),
    _("Sets buffersize for data retrieved from client"));
  menuClient->AppendSeparator();
  menuClient->Append(ID_TIMER_START, exEllipsed(_("Repeat Timer")),
    _("Repeats with timer writing last data to all clients"));
  menuClient->Append(ID_TIMER_STOP, _("Stop Timer"), _("Stops the timer"));
  menuClient->AppendSeparator();
  menuClient->Append(ID_WRITE_DATA, _("Write"), 
    _("Writes data to all clients"), wxITEM_NORMAL, NULL, wxART_GO_FORWARD);

  wxMenu* menuView = new wxMenu();
  menuView->Append(ID_VIEW_STATUSBAR, _("&Statusbar"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_TOOLBAR, _("&Toolbar"), wxEmptyString, wxITEM_CHECK);
  menuView->AppendSeparator();
  menuView->Append(ID_VIEW_LOG, _("Log"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_DATA, _("Data"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_STATISTICS, _("Statistics"), wxEmptyString, wxITEM_CHECK);

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(ID_OPTIONS, exEllipsed(_("Edit")));

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar* menuBar = new wxMenuBar();
  menuBar->Append(menuFile, _("&File"));
  menuBar->Append(menuView, _("&View"));
  menuBar->Append(menuServer, _("&Server"));
  menuBar->Append(menuClient, _("&Client"));
  menuBar->Append(menuOptions, _("&Options"));
  menuBar->Append(menuHelp, _("&Help"));
  SetMenuBar(menuBar);

  exToolBar* toolbar = new exToolBar(this,
    wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTB_FLAT | wxTB_NODIVIDER);
  toolbar->AddTool(wxID_NEW);
  toolbar->AddTool(wxID_OPEN);
  toolbar->AddTool(wxID_SAVE);
  toolbar->AddTool(
    ID_WRITE_DATA,
    wxEmptyString,
    wxArtProvider::GetBitmap(wxART_GO_FORWARD, wxART_TOOLBAR, toolbar->GetToolBitmapSize()),
    _("Write data"));
  toolbar->Realize();
  SetToolBar(toolbar);

  GetManager().AddPane(m_LogWindow,
    wxAuiPaneInfo().CenterPane().Name("LOG"));
  GetManager().AddPane(m_DataWindow,
    wxAuiPaneInfo().Hide().Left().MaximizeButton(true).Caption(_("Data")).Name("DATA"));
  GetManager().AddPane(m_Statistics.Show(this),
    wxAuiPaneInfo().Hide().Left().
      MaximizeButton(true).
      Caption(_("Statistics")).
      Name("STATISTICS"));

  if (SetupSocketServer())
  {
    Hide();
  }

  if (exApp::GetConfig(_("Timer"), 0) > 0)
  {
    m_Timer.Start(1000 * exApp::GetConfig(_("Timer"), 0));
    StatusText(wxString::Format("%ld", exApp::GetConfig(_("Timer"), 0)), "PaneTimer");
  }

  if (!GetRecentFile().empty())
  {
    OpenFile(GetRecentFile());
  }

  GetManager().LoadPerspective(exApp::GetConfig("Perspective"));
  GetManager().Update();
}

MyFrame::~MyFrame()
{
#ifdef USE_TASKBARICON
  delete m_TaskBarIcon;
#endif
  delete m_SocketServer;
}

void MyFrame::ConfigDialogApplied(wxWindowID /* dialogid */)
{
  m_LogWindow->ConfigGet();
  m_DataWindow->ConfigGet();
}

void MyFrame::LogConnection(
  wxSocketBase* sock,
  bool accepted,
  bool show_clients)
{
  wxString text;
  const wxString prefix = (accepted ? _("accepted"): _("lost"));

  text <<
    prefix << " " << SocketDetails(sock);

  if (show_clients)
  {
    text << " " << _("Clients: ") << m_Clients.size();
  }

  m_LogWindow->AppendTextWithTimestamp(text);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
  if (event.CanVeto())
  {
    Hide();
    return;
  }

  if (!m_DataWindow->Continue())
  {
    return;
  }

  exApp::SetConfig("Perspective", GetManager().SavePerspective());
  event.Skip();
}

void MyFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general socket server."));
    info.SetVersion("v1.0");
    info.SetCopyright("(c) 2007-2008, Anton van Wezenbeek");
    info.AddDeveloper(wxVERSION_STRING);
    info.AddDeveloper(EX_LIB_VERSION);
    info.AddDeveloper(FT_LIB_VERSION);
    wxAboutBox(info);
    }
    break;

  case wxID_EXIT:
    Close(true);
    break;

  case wxID_NEW:
    if (m_DataWindow->FileNew())
    {
      GetManager().GetPane("DATA").Show();
      GetManager().Update();
    }
    break;

  case wxID_OPEN:
    DialogFileOpen(wxFD_OPEN | wxFD_CHANGE_DIR, true);
    break;

  case wxID_SAVE:
    if (m_DataWindow->FileSave())
    {
      m_LogWindow->AppendTextWithTimestamp(
        _("saved: ") + m_DataWindow->GetFileName().GetFullPath());
    }
    break;

  case wxID_SAVEAS:
    if (m_DataWindow->FileSaveAs())
    {
      m_LogWindow->AppendTextWithTimestamp(
        _("saved: ") + m_DataWindow->GetFileName().GetFullPath());
    }
    break;

  case ID_CLEAR_STATISTICS:
    m_Statistics.Clear();
    break;

  case ID_CLIENT_BUFFER_SIZE:
    {
    long val;
    if ((val = wxGetNumberFromUser(
      _("Input:"),
      wxEmptyString,
      _("Buffer Size"),
      exApp::GetConfig(_("Buffer Size"), 4096),
      1,
      65536)) > 0)
    {
      exApp::SetConfig(_("Buffer Size"), val);
    }
    }
    break;

  case ID_CLIENT_ECHO:
    exApp::ToggleConfig(_("Echo"));
    break;

  case ID_CLIENT_LOG_DATA:
    exApp::ToggleConfig(_("Log Data"));
    break;

  case ID_HIDE:
    Close(false);
    break;

  case ID_OPTIONS:
    exSTC::ConfigDialog(_("Editor Options"),
      exSTC::STC_CONFIG_MODELESS | exSTC::STC_CONFIG_SIMPLE,
      this);
    break;

  case ID_SERVER_CONFIG:
    {
    std::vector<exConfigItem> v;
    v.push_back(exConfigItem(_("Hostname"), wxEmptyString, 0, true));
    v.push_back(exConfigItem(_("Port"), 1000, 65536));

    // Configuring only possible if server is stopped,
    // otherwise just show settings readonly mode.
    const long flags = (m_SocketServer == NULL ? wxOK|wxCANCEL: wxCANCEL);

    exConfigDialog(this,
      v,
      _("Server Config"),
      wxEmptyString,
      0,
      1,
      flags).ShowModal();
    }
    break;

  case ID_SERVER_START:
    SetupSocketServer();
    break;

  case ID_SERVER_STOP:
    {
      if (wxMessageBox(_("Stop server?"),
        _("Confirm"),
        wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
      {
        return;
      }

      for (
        std::list<wxSocketBase*>::iterator it = m_Clients.begin();
        it != m_Clients.end();
        ++it)
      {
        wxSocketBase* sock = *it;
        SocketLost(sock, false);
      }

      m_Clients.clear();

      m_SocketServer->Destroy();
      delete m_SocketServer;
      m_SocketServer = NULL;

      const wxString text = _("server stopped");
      
#ifdef USE_TASKBARICON
      m_TaskBarIcon->SetIcon(wxICON(notready), text);
#endif
      StatusText(
        wxString::Format(_("%d clients"), m_Clients.size()),
        "PaneClients");

      m_LogWindow->AppendTextWithTimestamp(text);

      const wxString statistics = m_Statistics.Get();

      if (!statistics.empty())
      {
        m_LogWindow->AppendTextWithTimestamp(statistics);
      }
    }
    break;

  case ID_TIMER_START: TimerDialog(); break;

  case ID_TIMER_STOP:
    m_Timer.Stop();
    m_LogWindow->AppendTextWithTimestamp(_("timer stopped"));
    StatusText(wxEmptyString, "PaneTimer");
    break;

  case ID_VIEW_DATA: TogglePane("DATA"); break;
  case ID_VIEW_LOG: TogglePane("LOG"); break;
  case ID_VIEW_STATISTICS: TogglePane("STATISTICS"); break;

  case ID_VIEW_STATUSBAR:
    GetStatusBar()->Show(!GetStatusBar()->IsShown());
    SendSizeEvent();
    break;

  case ID_VIEW_TOOLBAR:
    GetToolBar()->Show(!GetToolBar()->IsShown());
    SendSizeEvent();
    break;

  case ID_WRITE_DATA:
    WriteDataWindowToClients();
    break;

  default:
    wxLogError("Unhandled event");
  }
}

void MyFrame::OnSocket(wxSocketEvent& event)
{
  wxSocketBase *sock = event.GetSocket();

  if (event.GetId() == ID_SERVER)
  {
    m_Statistics.Inc(_("Socket Server Events"));

    // Accept new connection if there is one in the pending
    // connections queue, else exit. We use Accept(false) for
    // non-blocking accept (although if we got here, there
    // should ALWAYS be a pending connection).
    sock = m_SocketServer->Accept(false);

    if (sock == NULL)
    {
      m_LogWindow->AppendTextWithTimestamp(
        _("error: couldn't accept a new connection"));
      return;
    }

    m_Statistics.Inc(_("Socket Server Accepted Connections"));

    sock->SetEventHandler(*this, ID_CLIENT);
    sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    sock->Notify(true);

    m_Clients.push_back(sock);

    LogConnection(sock, true);

    wxString* buffer = m_DataWindow->GetTextRaw();
    WriteDataToClient(buffer, sock);
    delete buffer;

    const wxString text =
      wxString::Format(_("%s connected at %d"),
        wxTheApp->GetAppName().c_str(),
        exApp::GetConfig(_("Port"), 3000));

#ifdef USE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(connect), text);
#endif    
  }
  else if (event.GetId() == ID_CLIENT)
  {
    m_Statistics.Inc(_("Socket Client Events"));

    switch (event.GetSocketEvent())
    {
      case wxSOCKET_INPUT:
      {
        m_Statistics.Inc(_("Socket Client Input Events"));

        // We disable input events, so that the test doesn't trigger
        // wxSocketEvent again.
        sock->SetNotify(wxSOCKET_LOST_FLAG);

        const int size = exApp::GetConfig(_("Buffer Size"), 4096);

        if (size <= 0)
        {
          wxLogMessage("Illegal buffer size, skipping socket input data");
          return;
        }

        char* buffer = new char[size];
        sock->Read(buffer, size);

        m_Statistics.Inc(_("Bytes Received"), sock->LastCount());

        if (sock->LastCount() > 0)
        {
          if (exApp::GetConfigBool(_("Echo")))
          {
            sock->Write(buffer, sock->LastCount());
            SocketCheckError(sock);
            m_Statistics.Inc(_("Bytes Sent"), sock->LastCount());
          }

          if (exApp::GetConfigBool(_("Log Data")))
          {
            const wxString text(buffer, sock->LastCount());
            m_LogWindow->AppendTextWithTimestamp(
              _("read: '") + text + wxString::Format("' (%d bytes)", sock->LastCount()));
          }
        }

        delete [] buffer;

        StatusText(wxString::Format("%d,%d",
          m_Statistics.Get(_("Bytes Received")),
          m_Statistics.Get(_("Bytes Sent"))), 
          "PaneBytes");

        // Enable input events again.
        sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
        break;
      }

      case wxSOCKET_LOST:
        m_Statistics.Inc(_("Socket Client Lost Events"));
        SocketLost(sock, true);

        if (m_Clients.size() == 0)
        {
          const wxString text =
            wxString::Format(_("server listening at %d"),
              exApp::GetConfig(_("Port"), 3000));

#ifdef USE_TASKBARICON
          m_TaskBarIcon->SetIcon(wxICON(ready), text);
#endif
        }
        break;

      default: 
        m_Statistics.Inc(_("Socket Client Unhandled Events"));
    }
  }
  else
  {
    m_Statistics.Inc(_("Socket Unhandled Events"));
    wxLogError("Socket unhandled event");
  }

  StatusText(
    wxString::Format(_("%d clients"), m_Clients.size()),
    "PaneClients");
}

void MyFrame::OnTimer(wxTimerEvent& /* event */)
{
  WriteDataWindowToClients();
}

void MyFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case wxID_SAVE:
    event.Enable(m_DataWindow->GetModify());
    break;

  case ID_CLEAR_STATISTICS:
    event.Enable(!m_Statistics.GetItems().empty());
    break;

  case ID_CLIENT_ECHO:
    event.Check(exApp::GetConfigBool(_("Echo")));
    break;

  case ID_CLIENT_LOG_DATA:
    event.Check(exApp::GetConfigBool(_("Log Data")));
    break;

  case ID_RECENT_FILE_MENU: 
    event.Enable(!GetRecentFile().empty()); 
    break;

  case ID_SERVER_CONFIG:
    break;

  case ID_SERVER_STOP:
    event.Enable(m_SocketServer != NULL);
    break;

  case ID_SERVER_START:
    event.Enable(m_SocketServer == NULL);
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

  case ID_VIEW_STATISTICS:
    event.Check(GetManager().GetPane("STATISTICS").IsShown());
    break;

  case ID_VIEW_STATUSBAR:
    event.Check(GetStatusBar()->IsShown());
    break;

  case ID_VIEW_TOOLBAR:
    event.Check(GetToolBar()->IsShown());
    break;

  case ID_WRITE_DATA:
    event.Enable(m_Clients.size() > 0 && m_DataWindow->GetLength() > 0);
    break;

  default:
    wxLogError("Unhandled event");
  }
}

bool MyFrame::OpenFile(
  const wxString& file,
  int line_number,
  const wxString& match,
  long flags)
{
  if (m_DataWindow->Open(file, line_number, match, flags))
  {
    GetManager().GetPane("DATA").Show();
    GetManager().Update();

    m_LogWindow->AppendTextWithTimestamp(
      _("opened: ") + file + wxString::Format(" (%d bytes)",
      m_DataWindow->GetLength()));

    return true;
  }
  else
  {
    return false;
  }
}

bool MyFrame::SetupSocketServer()
{
  // Create the address - defaults to localhost and port as specified
  wxIPV4address addr;
  addr.Hostname(exApp::GetConfig(_("Hostname"), "localhost"));
  addr.Service(exApp::GetConfig(_("Port"), 3000));

  // Create the socket
  m_SocketServer = new wxSocketServer(addr);

  // We use Ok() here to see if the server is really listening
  if (!m_SocketServer->Ok())
  {
#ifdef USE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(notready), _("server not ready"));
#endif
    StatusText(_("Could not listen at the specified socket"));
    m_SocketServer->Destroy();
    delete m_SocketServer;
    m_SocketServer = NULL;
    return false;
  }
  else
  {
    const wxString text =
      wxString::Format(_("server listening at %d"), exApp::GetConfig(_("Port"), 3000));

#ifdef USE_TASKBARICON
    m_TaskBarIcon->SetIcon(wxICON(ready), text);
#endif    
    StatusText(text);
    m_LogWindow->AppendTextWithTimestamp(text);
  }

  // Setup the event handler and subscribe to connection events
  m_SocketServer->SetEventHandler(*this, ID_SERVER);
  m_SocketServer->SetNotify(wxSOCKET_CONNECTION_FLAG);
  m_SocketServer->Notify(true);

  return true;
}

void MyFrame::SocketCheckError(wxSocketBase* sock)
{
  if (sock->Error())
  {
    const wxString error = wxString::Format(_("Socket error: %d"), sock->LastError());
    StatusText(error);
    m_Statistics.Inc(error);
  }
}

const wxString MyFrame::SocketDetails(wxSocketBase* sock) const
{
  wxString localIPAddress = "";
  wxString peerIPAddress = "";
  int localPort = 0;
  int peerPort = 0;

  wxIPV4address addr;

  if (sock != NULL)
  {
    if (sock->GetPeer(addr))
    {
      peerIPAddress = addr.IPAddress();
      peerPort = addr.Service();
    }

    if (sock->GetLocal(addr))
    {
      localIPAddress = addr.IPAddress();
      localPort = addr.Service();
    }
  }

  wxString value;

  value <<
    _("socket: ") <<
    localIPAddress << "." << localPort << ", " <<
    peerIPAddress << "." << peerPort;

  return value;
}

void MyFrame::SocketLost(wxSocketBase* sock, bool remove_from_clients)
{
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

void MyFrame::StatusBarDoubleClicked(int field, const wxPoint& point)
{
  if (field == GetPaneField("PaneTimer"))
  {
    TimerDialog();
  }
  else
  {
    ftFrame::StatusBarDoubleClicked(field, point);
  }
}

void MyFrame::TimerDialog()
{
  const long val = wxGetNumberFromUser(
    _("Input:"),
    wxEmptyString,
    _("Give Seconds"),
    exApp::GetConfig(_("Timer"), 4096),
    1,
    3600 * 24);

  exApp::SetConfig(_("Timer"), val);

  if (val > 0)
  {
    m_Timer.Start(1000 * val);
    m_LogWindow->AppendTextWithTimestamp(
      wxString::Format(_("timer set to: %d seconds (%s)"),
      val,
      wxTimeSpan(0, 0, val, 0).Format().c_str()));
    StatusText(wxString::Format("%ld", val), "PaneTimer");
  }
  else if (val == 0)
  {
    m_Timer.Stop();
    m_LogWindow->AppendTextWithTimestamp(_("timer stopped"));
    StatusText(wxEmptyString, "PaneTimer");
  }
}

void MyFrame::WriteDataToClient(wxString* buffer, wxSocketBase* client)
{
  if (buffer->empty()) return;

  client->Write((*buffer).c_str(), buffer->size());

  SocketCheckError(client);

  if (client->LastCount() != buffer->size())
  {
    m_LogWindow->AppendTextWithTimestamp(_("not all bytes sent to socket"));
  }

  m_Statistics.Inc(_("Messages Sent"));

  StatusText(wxString::Format("%d,%d",
    m_Statistics.Get(_("Bytes Received")),
    m_Statistics.Inc(_("Bytes Sent"), client->LastCount())),
    "PaneBytes");

  if (exApp::GetConfigBool(_("Log Data")))
  {
    m_LogWindow->AppendTextWithTimestamp(
      wxString::Format(_("write: %d bytes to %s"),
        client->LastCount(), SocketDetails(client).c_str()));
  }
}

void MyFrame::WriteDataWindowToClients()
{
  wxString* buffer = m_DataWindow->GetTextRaw();

  for (
    std::list<wxSocketBase*>::iterator it = m_Clients.begin();
    it != m_Clients.end();
    ++it)
  {
    wxSocketBase* sock = *it;
    WriteDataToClient(buffer, sock);
  }

  delete buffer;
}

#ifdef USE_TASKBARICON
enum
{
  ID_OPEN = ID_CLIENT + 1,
};

BEGIN_EVENT_TABLE(MyTaskBarIcon, wxTaskBarIcon)
  EVT_MENU(wxID_EXIT, MyTaskBarIcon::OnCommand)
  EVT_MENU(ID_OPEN, MyTaskBarIcon::OnCommand)
  EVT_TASKBAR_LEFT_DCLICK(MyTaskBarIcon::OnTaskBarIcon)
  EVT_UPDATE_UI(wxID_EXIT, MyTaskBarIcon::OnUpdateUI)
END_EVENT_TABLE()

MyTaskBarIcon::MyTaskBarIcon(MyFrame* frame)
  : m_Frame(frame)
{
}

wxMenu *MyTaskBarIcon::CreatePopupMenu()
{
  exMenu* menu = new exMenu;
  menu->Append(ID_OPEN, _("Open"));
  menu->AppendSeparator();
  menu->Append(wxID_EXIT);
  return menu;
}

void MyTaskBarIcon::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_EXIT:
    m_Frame->Close(true);
    break;
  case ID_OPEN:
    m_Frame->Show();
    break;
  }
}

void MyTaskBarIcon::OnTaskBarIcon(wxTaskBarIconEvent&)
{
  m_Frame->Show();
}

void MyTaskBarIcon::OnUpdateUI(wxUpdateUIEvent& event)
{
  event.Enable(m_Frame->ServerNotListening());
}
#endif
