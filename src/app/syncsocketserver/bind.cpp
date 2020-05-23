////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of bind_all for syncsocketserver
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <functional>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "app.h"
#include <wex/bind.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/filedlg.h>
#include <wex/itemdlg.h>
#include <wex/util.h>
#include <wx/numdlg.h>

#ifdef __WXOSX__
#undef wxUSE_TASKBARICON
#endif

#ifdef __WXGTK__
#undef wxUSE_TASKBARICON
#endif

void frame::bind_all()
{
  Bind(
    wxEVT_MENU,
    [=](wxCommandEvent& event) {
      const std::string buffer(event.GetString());
      size_t            written = 0;

      for (auto& it : m_clients)
      {
        written += write_data_to_socket(buffer + "\n", it);
      }
      if (m_client != nullptr)
      {
        written += write_data_to_socket(buffer + "\n", m_client);
      }

      if (written == 0)
      {
        wex::log::verbose("ignored") << buffer;
      }

      m_shell->prompt();
    },
    wex::ID_SHELL_COMMAND);

  Bind(
    wxEVT_MENU,
    [=](wxCommandEvent& event) {
      m_log->clear();
    },
    m_id_clear_log);

  Bind(
    wxEVT_SOCKET,
    [=](wxSocketEvent& event) {
      if (wxSocketBase* sock = m_server->Accept(false); sock == nullptr)
      {
        wex::log() << "couldn't accept a new connection";
      }
      else
      {
        m_stats.inc("Connections Server");
        sock->SetEventHandler(*this, m_id_socket_client);
        sock->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
        sock->Notify(true);
        m_clients.emplace_back(sock);
        log_connection(sock, true);

        if (const auto& buffer = m_data->get_text(); !buffer.empty())
        {
          write_data_to_socket(buffer, sock);
        }
#if wxUSE_TASKBARICON
        update_taskbar();
#endif
      };
    },
    m_id_socket_server);

  Bind(
    wxEVT_SOCKET,
    [=](wxSocketEvent& event) {
      wxSocketBase* sock = event.GetSocket();

      switch (event.GetSocketEvent())
      {
        case wxSOCKET_CONNECTION:
        {
          m_stats.inc("Connections Remote");
          log_connection(sock, true);
          sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);
          if (const auto& buffer = m_data->get_text(); !buffer.empty())
          {
            write_data_to_socket(buffer, sock);
          }
        }
        break;

        case wxSOCKET_INPUT:
        {
          // We disable input events, so that the test doesn't trigger
          // wxSocketEvent again.
          sock->SetNotify(wxSOCKET_LOST_FLAG);

          const long size   = wex::config("Buffer Size").get(4096);
          char*      buffer = new char[size];
          sock->Read(buffer, size);

          if (sock->LastReadCount() > 0)
          {
            m_stats.inc("Messages Received");
            m_stats.inc("Bytes Received", sock->LastReadCount());

            const std::string text(buffer, sock->LastReadCount());

            if (wex::config("Log Data").get(true))
            {
              if (wex::config("Count Only").get(true))
              {
                append_text(
                  m_log,
                  "read " + std::to_string(sock->LastReadCount()) +
                    " bytes from " + socket_details(sock),
                  data_mode_t::MESSAGE);
              }
              else
              {
                append_text(m_log, text, data_mode_t::READ);
              }
            }

            wex::log::verbose("read") << text;

            switch (m_answer)
            {
              case answer_t::COMMAND:
                if (m_shell->get_command() != "history")
                {
                  write_data_to_socket(m_shell->get_command(), sock);
                }
                break;

              case answer_t::ECHO:
                write_data_to_socket(text, sock);
                break;

              case answer_t::FILE:
                if (const auto& b(m_data->get_text()); !b.empty())
                {
                  write_data_to_socket(b, sock);
                }
                break;

              default:
                break;
            }

            if (pane_is_shown("SHELL"))
            {
              append_text(m_shell, text, data_mode_t::MESSAGE_RAW);
              m_shell->prompt(std::string(), false); // no eol
            }
          }

          statustext(
            std::to_string(m_stats.get("Bytes Received")) + "," +
              std::to_string(m_stats.get("Bytes Sent")),
            "PaneBytes");

#if wxUSE_TASKBARICON
          update_taskbar();
#endif
          // Enable input events again.
          sock->SetNotify(wxSOCKET_LOST_FLAG | wxSOCKET_INPUT_FLAG);

          delete[] buffer;
        }
        break;

        case wxSOCKET_LOST:
          m_stats.inc("Connections Closed");

          if (event.GetId() == m_id_socket_client)
          {
            socket_closed(sock, true);
          }
          else
          {
            socket_closed(sock, false);
            m_client = nullptr;
          }

          update_connections_pane();

#if wxUSE_TASKBARICON
          update_taskbar();
#endif
          break;

        default:
          assert(0);
      };
    },
    m_id_socket_client,
    m_id_socket_remoteclient);

  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
#if wxUSE_TASKBARICON
    if (event.CanVeto())
    {
      Hide();
      return;
    }
#endif
    if (
      wex::file_dialog(&m_data->get_file()).show_modal_if_changed() ==
      wxID_CANCEL)
    {
      return;
    }

    if (m_client != nullptr)
    {
      m_client->Destroy();
      m_client = nullptr;
    }

    for (auto c : m_clients)
      c->Destroy();
    m_clients.clear();

    event.Skip();
  });

  Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
    write_data_window_to_connections();
  });

  wex::bind(this).ui({{[=](wxUpdateUIEvent& event) {
                         event.Enable(m_server == nullptr);
                       },
                       wxID_EXECUTE},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(m_data->GetModify());
                       },
                       wxID_SAVE},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(m_server != nullptr);
                       },
                       wxID_STOP},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(m_client == nullptr);
                       },
                       m_id_remote_server_connect},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(m_client != nullptr);
                       },
                       m_id_remote_server_disconnect},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(!m_stats.get_items().empty());
                       },
                       m_id_clear_statistics},
                      {[=](wxUpdateUIEvent& event) {
                         event.Check(wex::config("Log Data").get(true));
                       },
                       m_id_client_log_data},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(wex::config("Log Data").get(true));
                         event.Check(wex::config("Count Only").get(true));
                       },
                       m_id_client_log_data_count_only},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(
                           !file_history().get_history_file().empty());
                       },
                       m_id_recent_file_menu},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(m_timer.IsRunning());
                       },
                       m_id_timer_stop},
                      {[=](wxUpdateUIEvent& event) {
                         event.Check(pane_is_shown("DATA"));
                       },
                       m_id_view_data},
                      {[=](wxUpdateUIEvent& event) {
                         event.Check(pane_is_shown("LOG"));
                       },
                       m_id_view_log},
                      {[=](wxUpdateUIEvent& event) {
                         event.Check(pane_is_shown("SHELL"));
                       },
                       m_id_view_shell},
                      {[=](wxUpdateUIEvent& event) {
                         event.Check(pane_is_shown("STATISTICS"));
                       },
                       m_id_view_statistics},
                      {[=](wxUpdateUIEvent& event) {
                         event.Enable(
                           (!m_clients.empty() ||
                            (m_client != nullptr && m_client->IsConnected())) &&
                           m_data->GetLength() > 0);
                       },
                       m_id_write_data}});
}
