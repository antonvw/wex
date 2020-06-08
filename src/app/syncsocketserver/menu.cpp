////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of frame::build_menu
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "app.h"
#include <wex/config.h>
#include <wex/core.h>
#include <wex/defs.h>
#include <wex/filedlg.h>
#include <wex/itemdlg.h>
#include <wex/log.h>
#include <wex/menubar.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/version.h>
#include <wx/aboutdlg.h>
#include <wx/numdlg.h>

#ifdef __WXOSX__
#undef wxUSE_TASKBARICON
#endif

#ifdef __WXGTK__
#undef wxUSE_TASKBARICON
#endif

void frame::build_menu()
{
  auto* menuFile = new wex::menu({
    {wxID_NEW, "", wex::data::menu().action([=](wxCommandEvent& event) {
       m_data->get_file().file_new(wex::path());
       pane_show("DATA");
     })},
      {wxID_OPEN, "", wex::data::menu().action([=](wxCommandEvent& event) {
         wex::data::window data;
         data.style(wxFD_OPEN | wxFD_CHANGE_DIR);
         wex::open_files_dialog(this, true, wex::data::stc(data));
       })},
      {}, {m_id_recent_file_menu, file_history()},
      {wxID_SAVE, "", wex::data::menu().action([=](wxCommandEvent& event) {
         m_data->get_file().file_save();
       })},
      {wxID_SAVEAS, "", wex::data::menu().action([=](wxCommandEvent& event) {
         wex::file_dialog dlg(
           &m_data->get_file(),
           wex::data::window().style(wxFD_SAVE).parent(this).title(
             wxGetStockLabel(wxID_SAVEAS).ToStdString()));
         if (dlg.ShowModal())
         {
           m_data->get_file().file_save(dlg.GetPath().ToStdString());
         }
       })},
      {},
      {m_id_clear_statistics,
       "Clear Statistics",
       wex::data::menu()
         .help_text("Clears the statistics")
         .action([=](wxCommandEvent& event) {
           m_stats.clear();
         })},
      {},
#if wxUSE_TASKBARICON
      {m_id_hide,
       "Hide",
       wex::data::menu()
         .help_text("Puts back in the task bar")
         .action([=](wxCommandEvent& event) {
           Close(false);
         })},

#else
    {
      wex::menu_item::EXIT
    }
  });
#endif

#ifndef __WXOSX__
      auto* menuOptions = new wex::menu({{wxID_PREFERENCES, std::string()}});
#else
  menuFile->append({{wxID_PREFERENCES}}); // is moved!
#endif

    SetMenuBar(new wex::menubar(
      {{menuFile, wxGetStockLabel(wxID_FILE)},
       {new wex::menu({{this},
                       {},
                       {m_id_view_log,
                        "Log",
                        wex::menu_item::CHECK,
                        wex::data::menu().action([=](wxCommandEvent& event) {
                          pane_toggle("LOG");
                        })},
                       {m_id_view_data,
                        "Data",
                        wex::menu_item::CHECK,
                        wex::data::menu().action([=](wxCommandEvent& event) {
                          pane_toggle("DATA");
                        })},
                       {m_id_view_shell,
                        "Shell",
                        wex::menu_item::CHECK,
                        wex::data::menu().action([=](wxCommandEvent& event) {
                          pane_toggle("SHELL");
                        })},
                       {m_id_view_statistics,
                        "Statistics",
                        wex::menu_item::CHECK,
                        wex::data::menu().action([=](wxCommandEvent& event) {
                          pane_toggle("STATISTICS");
                        })}}),
        "&View"},
       {new wex::menu(
          {{m_id_server_config,
            wex::ellipsed("Configuration"),
            wex::data::menu()
              .help_text("Configures the server")
              .action([=](wxCommandEvent& event) {
                // Configuring only possible if server is stopped,
                // otherwise just show settings readonly mode.
                wex::item_dialog(
                  {{"Hostname",
                    std::string(),
                    wex::item::TEXTCTRL,
                    wex::data::control().is_required(true)},
                   // Well known ports are in the range from 0 to
                   // 1023. Just allow here for most flexibility.
                   {"Port", 1, 65536, 3000}},
                  wex::data::window()
                    .title("Server Config")
                    .button(m_server == nullptr ? wxOK | wxCANCEL : wxCANCEL))
                  .ShowModal();
              })},
           {},
           {wxID_EXECUTE,
            "",
            wex::data::menu().action([=](wxCommandEvent& event) {
              setup_server();
            })},
           {wxID_STOP, "", wex::data::menu().action([=](wxCommandEvent& event) {
              if (
                wxMessageBox(
                  "Stop server?",
                  "Confirm",
                  wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
              {
                return;
              }
              const std::string text("server stopped");
              wex::log::status(text);
              wex::log::verbose("server") << "stopped";
              append_text(m_log, text, data_mode_t::MESSAGE);
              for (auto& it : m_clients)
              {
                socket_closed(it, false);
              }
              m_clients.clear();
              m_server->Destroy();
              m_server = nullptr;
#if wxUSE_TASKBARICON
              m_taskbar_icon->SetIcon(wxICON(notready), text);
#endif
              update_connections_pane();
            })},
           {},
           {new wex::menu(
              {{m_id_remote_server_config,
                wex::ellipsed("Configuration"),
                wex::data::menu()
                  .help_text("Configures the remote server")
                  .action([=](wxCommandEvent& event) {
                    // Configuring only possible if no client is
                    // active, otherwise just show settings
                    // readonly mode.
                    wex::item_dialog(
                      {{"Remote Hostname",
                        wex::item::COMBOBOX,
                        std::any(),
                        wex::data::control().is_required(true)},
                       // Well known ports are in the range from 0
                       // to 1023. Just allow here for most
                       // flexibility.
                       {"Remote Port", 1, 65536, 3000}},
                      wex::data::window()
                        .title("Remote Server Config")
                        .button(
                          m_client == nullptr ? wxOK | wxCANCEL : wxCANCEL))
                      .ShowModal();
                  })},
               {},
               {m_id_remote_server_connect,
                "Connect",
                wex::data::menu()
                  .help_text("Tries to connect to server")
                  .action([=](wxCommandEvent& event) {
                    assert(m_client == nullptr);

                    wxIPV4address addr;
                    addr.Hostname(
                      wex::config("Remote Hostname").get("localhost"));
                    addr.Service(wex::config("Remote Port").get(3000));

                    m_client = new wxSocketClient();
                    m_client->SetEventHandler(*this, m_id_socket_remoteclient);
                    m_client->SetNotify(wxSOCKET_CONNECTION_FLAG);
                    m_client->Notify(true);
                    m_client->Connect(addr, false);
                  })},
               {m_id_remote_server_disconnect,
                "Disconnect",
                wex::data::menu()
                  .help_text("Disconnects from remote server")
                  .action([=](wxCommandEvent& event) {
                    assert(m_client != nullptr);

                    if (m_client->IsConnected())
                    {
                      socket_closed(m_client, false);
                    }

                    update_connections_pane();

                    m_client = nullptr;
                  })}}),
            "&Remote"}}),
        "&Server"},
       {new wex::menu(
          {{new wex::menu(
              {{m_id_client_answer_off,
                "Off",
                wex::menu_item::RADIO,
                wex::data::menu()
                  .action([=](wxCommandEvent& event) {
                    m_answer = answer_t::OFF;
                  })
                  .help_text("No answer back to connection")},
               {m_id_client_answer_echo,
                "Echo",
                wex::menu_item::RADIO,
                wex::data::menu()
                  .action([=](wxCommandEvent& event) {
                    m_answer = answer_t::ECHO;
                  })
                  .help_text("Echo's received data back to connection")},
               {m_id_client_answer_command,
                "Command",
                wex::menu_item::RADIO,
                wex::data::menu()
                  .action([=](wxCommandEvent& event) {
                    m_answer = answer_t::COMMAND;
                  })
                  .help_text("Send last shell command back to connection")},
               {m_id_client_answer_file,
                "File",
                wex::menu_item::RADIO,
                wex::data::menu()
                  .action([=](wxCommandEvent& event) {
                    m_answer = answer_t::FILE;
                  })
                  .help_text("Send file contents back to connection")}}),
            "&Answer"},
           {},
           {m_id_client_log_data,
            "Log Data",
            wex::menu_item::CHECK,
            wex::data::menu()
              .action([=](wxCommandEvent& event) {
                wex::config("Log Data").set(!wex::config("Log Data").get(true));
              })
              .help_text("Logs data read from and written to connection")},
           {m_id_client_log_data_count_only,
            "Count Only",
            wex::menu_item::CHECK,
            wex::data::menu()
              .action([=](wxCommandEvent& event) {
                wex::config("Count Only")
                  .set(!wex::config("Count Only").get(true));
              })
              .help_text("Logs only byte counts, no text")},
           {},
           {m_id_client_buffer_size,
            wex::ellipsed("Buffer Size"),
            wex::data::menu()
              .help_text("Sets buffersize for data retrieved from connection")
              .action([=](wxCommandEvent& event) {
                if (const auto val(wxGetNumberFromUser(
                      "Input:",
                      wxEmptyString,
                      "Buffer Size",
                      wex::config("Buffer Size").get(4096),
                      1,
                      65536));
                    val > 0)
                {
                  wex::config("Buffer Size").set(val);
                }
              })},
           {},
           {m_id_timer_start,
            wex::ellipsed("Repeat Timer"),
            wex::data::menu()
              .help_text("Repeats with timer writing last data to all "
                         "connections")
              .action([=](wxCommandEvent& event) {
                timer_dialog();
              })},
           {m_id_timer_stop,
            "Stop Timer",
            wex::data::menu()
              .help_text("Stops the timer")
              .action([=](wxCommandEvent& event) {
                m_timer.Stop();
                wex::log::verbose("timer") << "stopped";
                statustext(std::string(), "PaneTimer");
              })},
           {},
           {m_id_write_data,
            "Write",
            wex::data::menu()
              .help_text("Writes data to all connections")
              .art(wxART_GO_FORWARD)
              .action([=](wxCommandEvent& event) {
                write_data_window_to_connections();
              })}}),
        "&Connection"},
#ifndef __WXOSX__
       {menuOptions, "&Options"},
#endif
       {new wex::menu(
          {{wxID_ABOUT, "", wex::data::menu().action([=](wxCommandEvent& event) {
              wxAboutDialogInfo info;
              info.SetIcon(GetIcon());
              info.SetDescription("This program offers a general socket "
                                  "server.");
              info.SetVersion(wex::get_version_info().get());
              info.SetCopyright(wex::get_version_info().copyright());
              wxAboutBox(info);
            })}}),
        wxGetStockLabel(wxID_HELP)}}));
}
