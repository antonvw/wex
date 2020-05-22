////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of classes for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wex/app.h>
#include <wex/report/frame.h>
#include <wex/shell.h>
#include <wex/statistics.h>
#include <wex/stc.h>
#include <wx/socket.h>
#include <wx/taskbar.h>

#if wxUSE_TASKBARICON
class taskbar_icon;
#endif

class app : public wex::app
{
private:
  bool OnInit() override;
};

class frame : public wex::report::frame
{
public:
  frame();
  ~frame();
  bool server_not_listening() const { return m_server == nullptr; }

private:
  enum class answer_t
  {
    OFF,
    COMMAND,
    ECHO,
    FILE,
  };

  enum class data_mode_t
  {
    MESSAGE,
    MESSAGE_RAW,
    READ,
    WRITE,
  };

  void on_command_item_dialog(wxWindowID dialogid, const wxCommandEvent& event)
    override;

  wex::stc* open_file(
    const wex::path&     filename,
    const wex::stc_data& data = wex::stc_data()) override;

  void statusbar_clicked(const std::string& pane) override;

  void append_text(wex::stc* stc, const std::string& text, data_mode_t mode);

  void bind_all();

  void log_connection(
    wxSocketBase* sock,
    bool          accepted     = true,
    bool          show_clients = true);

  bool setup_server();

  void socket_closed(wxSocketBase* sock, bool remove_from_clients);

  const std::string socket_details(const wxSocketBase* sock) const;

  void timer_dialog();

#if wxUSE_TASKBARICON
  void update_taskbar();
#endif

  void update_connections_pane() const;

  size_t write_data_to_socket(const std::string& data, wxSocketBase* client);

  size_t write_data_window_to_connections();

  std::list<wxSocketBase*> m_clients;

  wex::shell* m_shell;
  wex::stc *  m_data, *m_log;

  wex::statistics<int> m_stats{{{"Messages Received", 0},
                                {"Messages Sent", 0},
                                {"Bytes Received", 0},
                                {"Bytes Sent", 0},
                                {"Connections Server", 0},
                                {"Connections Remote", 0},
                                {"Connections Closed", 0}}};

  wxSocketClient* m_client{nullptr};
  wxSocketServer* m_server{nullptr};
  wxTimer         m_timer;

#if wxUSE_TASKBARICON
  taskbar_icon* m_taskbar_icon;
#endif

  answer_t m_answer{answer_t::OFF};
};

#if wxUSE_TASKBARICON
class taskbar_icon : public wxTaskBarIcon
{
public:
  explicit taskbar_icon(frame* frame);

private:
  wxMenu* CreatePopupMenu() override;
  frame*  m_frame;
};
#endif
