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
    const wxExFileName& filename, 
    int line_number = 0,
    const std::string& match = std::string(), 
    int col_number = 0,
    wxExSTCWindowFlags flags = STC_WIN_DEFAULT, 
    const std::string& command = std::string()) override;
  virtual void StatusBarClicked(const std::string& pane) override;

  void LogConnection(
    wxSocketBase* sock,
    bool accepted = true,
    bool show_clients = true);
  bool SetupSocketServer();
  bool SocketCheckError(const wxSocketBase* sock);
  const wxString SocketDetails(const wxSocketBase* sock) const;
  void SocketLost(wxSocketBase* sock, bool remove_from_clients);
  void TimerDialog();
#if wxUSE_TASKBARICON
  void UpdateTaskBar();
#endif
  void WriteDataToClient(const wxCharBuffer& data, wxSocketBase* client);
  void WriteDataWindowToClients();

  std::list<wxSocketBase*> m_Clients;

  wxExSTC* m_DataWindow;
  wxExSTC* m_LogWindow;
  wxExShell* m_Shell;

  wxExStatistics < int > m_Statistics;

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
protected:
  virtual wxMenu* CreatePopupMenu();
private:
  Frame* m_Frame;
};
#endif

enum
{
  ID_MENU_FIRST,

  ID_CLEAR_STATISTICS,
  ID_CLIENT_BUFFER_SIZE,
  ID_CLIENT_ANSWER_COMMAND,
  ID_CLIENT_ANSWER_ECHO,
  ID_CLIENT_ANSWER_FILE,
  ID_CLIENT_ANSWER_OFF,
  ID_CLIENT_LOG_DATA,
  ID_CLIENT_LOG_DATA_COUNT_ONLY,
  ID_HIDE,
  ID_RECENT_FILE_MENU,
  ID_TIMER_STOP,
  ID_TIMER_START,
  ID_VIEW_DATA,
  ID_VIEW_LOG,
  ID_VIEW_SHELL,
  ID_VIEW_STATISTICS,
  ID_WRITE_DATA,

  ID_MENU_LAST,

  ID_SERVER,
  ID_CLIENT
};

#endif // wxUSE_SOCKETS
