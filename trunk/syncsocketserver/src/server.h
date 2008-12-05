/******************************************************************************\
* File:          server.h
* Purpose:       General socket server
* Author:        Anton van Wezenbeek
*
* Copyright (c) 2007-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/socket.h>
#include <wx/taskbar.h>
#include <wx/filetool/filetool.h>

class MyTaskBarIcon;

class MyApp : public exApp
{
public:
  virtual bool OnInit();
};

class MyFrame : public ftFrame
{
public:
  MyFrame();
 ~MyFrame();
  bool ServerNotListening() const {
    return m_SocketServer == NULL;}
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnSocket(wxSocketEvent& event);
  void OnTimer(wxTimerEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual void ConfigDialogApplied(wxWindowID dialogid);
  virtual bool OpenFile(const wxString& file,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
  virtual void StatusBarDoubleClicked(int field, const wxPoint& point);

  void LogConnection(
    wxSocketBase* sock, 
    bool accepted = true, 
    bool show_clients = true);
  bool SetupSocketServer();
  void SocketCheckError(wxSocketBase* sock);
  const wxString SocketDetails(wxSocketBase* sock) const;
  void SocketLost(wxSocketBase* sock, bool remove_from_clients);
  void TimerDialog();
  void WriteDataToClient(wxString* data, wxSocketBase* client);
  void WriteDataWindowToClients();

  std::list<wxSocketBase*> m_Clients;

  ftSTC* m_LogWindow;
  ftSTC* m_DataWindow;

  exStatistics < long > m_Statistics;

  wxSocketServer* m_SocketServer;
  wxTimer m_Timer;

  MyTaskBarIcon* m_TaskBarIcon;

  DECLARE_EVENT_TABLE()
};

class MyTaskBarIcon: public wxTaskBarIcon
{
public:
  MyTaskBarIcon(MyFrame* frame);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnTaskBarIcon(wxTaskBarIconEvent&);
  void OnUpdateUI(wxUpdateUIEvent&);
  virtual wxMenu* CreatePopupMenu();
private:
  MyFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

enum
{
  ID_MENU_FIRST,

  ID_CLEAR_STATISTICS,
  ID_CLIENT_BUFFER_SIZE,
  ID_CLIENT_ECHO,
  ID_CLIENT_LOG_DATA,
  ID_HIDE,
  ID_OPTIONS,
  ID_RECENT_FILE_MENU,
  ID_SERVER_START,
  ID_SERVER_STOP,
  ID_SERVER_CONFIG,
  ID_TIMER_STOP,
  ID_TIMER_START,
  ID_VIEW_DATA,
  ID_VIEW_LOG,
  ID_VIEW_STATISTICS,
  ID_VIEW_STATUSBAR,
  ID_VIEW_TOOLBAR,
  ID_WRITE_DATA,

  ID_MENU_LAST,

  ID_SERVER,
  ID_CLIENT,
};
