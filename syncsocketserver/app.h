////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/socket.h>
#include <wx/taskbar.h>
#include <wex/app.h>
#include <wex/shell.h>
#include <wex/stc.h>
#include <wex/report/frame.h>

#if wxUSE_SOCKETS

#if wxUSE_TASKBARICON
class TaskBarIcon;
#endif

class app : public wex::app
{
private:
  virtual bool OnInit() override;
};

class frame : public wex::history_frame
{
public:
  frame();
 ~frame();
  bool ServerNotListening() const {
    return m_SocketServer == nullptr;}
private:
  void AppendText(
    wex::stc* stc, 
    const wxString& text,
    int mode);
  virtual void on_command_item_dialog(
    wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wex::stc* open_file(
    const wex::path& filename, 
    const wex::stc_data& data = wex::stc_data()) override;
  virtual void statusbar_clicked(const std::string& pane) override;

  void LogConnection(
    wxSocketBase* sock,
    bool accepted = true,
    bool show_clients = true);
  bool SetupSocketServer();
  const wxString SocketDetails(const wxSocketBase* sock) const;
  void SocketClosed(wxSocketBase* sock, bool remove_from_clients);
  void TimerDialog();
#if wxUSE_TASKBARICON
  void UpdateTaskBar();
#endif
  void UpdateConnectionsPane() const;
  void WriteDataToSocket(const wxCharBuffer& data, wxSocketBase* client);
  void WriteDataWindowToConnections();

  std::list<wxSocketBase*> m_Clients;

  wex::stc* m_DataWindow;
  wex::stc* m_LogWindow;
  wex::shell* m_Shell;

  wex::statistics < int > m_Statistics {{
    {"Messages Received", 0},
    {"Messages Sent", 0},
    {"Bytes Received", 0},
    {"Bytes Sent", 0},
    {"Connections Server", 0},
    {"Connections Remote", 0},
    {"Connections Closed", 0}}};

  wxSocketClient* m_SocketRemoteClient {nullptr};
  wxSocketServer* m_SocketServer {nullptr};
  wxTimer m_Timer;

#if wxUSE_TASKBARICON
  TaskBarIcon* m_TaskBarIcon;
#endif

  int m_Answer;
};

#if wxUSE_TASKBARICON
class TaskBarIcon: public wxTaskBarIcon
{
public:
  explicit TaskBarIcon(frame* frame);
private:
  virtual wxMenu* CreatePopupMenu() override;
  frame* m_Frame;
};
#endif

#endif // wxUSE_SOCKETS
