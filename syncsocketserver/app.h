////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/socket.h>
#include <wx/taskbar.h>
#include <wx/extension/app.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/report/frame.h>

#if wxUSE_SOCKETS

#if wxUSE_TASKBARICON
class TaskBarIcon;
#endif

class App : public wxExApp
{
private:
  virtual bool OnInit() override;
};

class Frame : public wxExFrameWithHistory
{
public:
  Frame();
 ~Frame();
  bool ServerNotListening() const {
    return m_SocketServer == nullptr;}
private:
  void AppendText(
    wxExSTC* stc, 
    const wxString& text,
    int mode);
  virtual void OnCommandItemDialog(
    wxWindowID dialogid, const wxCommandEvent& event) override;
  virtual wxExSTC* OpenFile(
    const wxExPath& filename, 
    const wxExSTCData& data = wxExSTCData()) override;
  virtual void StatusBarClicked(const std::string& pane) override;

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

  wxExSTC* m_DataWindow;
  wxExSTC* m_LogWindow;
  wxExShell* m_Shell;

  wxExStatistics < int > m_Statistics;

  wxSocketClient* m_SocketRemoteClient = nullptr;
  wxSocketServer* m_SocketServer = nullptr;
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
  explicit TaskBarIcon(Frame* frame);
private:
  virtual wxMenu* CreatePopupMenu() override;
  Frame* m_Frame;
};
#endif

#endif // wxUSE_SOCKETS
