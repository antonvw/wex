////////////////////////////////////////////////////////////////////////////////
// Name:      frame-dialog.cpp
// Purpose:   Implementation of wex::del::frame class dialog methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

wex::stc_entry_dialog*
wex::del::frame::entry_dialog(const std::string& title, const std::string& text)
{
  if (m_entry_dialog == nullptr)
  {
    m_entry_dialog = new stc_entry_dialog(
      text,
      std::string(),
      data::window().title(title),
      data::stc(data::window().size({350, 250})));
  }
  else
  {
    if (!text.empty())
    {
      m_entry_dialog->get_stc()->set_text(text);
    }

    if (!title.empty())
    {
      m_entry_dialog->SetTitle(title);
    }
  }

  return m_entry_dialog;
}

int wex::del::frame::lsp_config_dialog(const data::window& par)
{
  item::choices_bool_t choices;

  // name lsp server, with pair whether enabled and lexer name
  std::map<std::string, std::pair<bool, std::string>> servers_1;

  for (const auto& server : lexers::get()->get_lsp_servers())
  {
    choices.insert(server.first);

    servers_1.emplace(
      server.first,
      std::make_pair(config(server.first).get(false), server.second));
  }

  if (m_lsp_dialog == nullptr)
  {
    m_lsp_dialog = new item_dialog(
      std::vector<item>{{choices}},
      data::window(par).title(_("Set LSP Server").ToStdString()));
  }
  else
  {
    m_lsp_dialog->reload();
  }

  const int result(m_lsp_dialog->ShowModal());

  std::map<std::string, bool> servers_2;

  for (const auto& server : lexers::get()->get_lsp_servers())
  {
    servers_2.emplace(server.first, config(server.first).get(false));
  }

  // so, if lsp server changed in config, shutdown if it was enabled,
  // and add a client if it was disabled
  for (auto& server : servers_1)
  {
    if (server.second.first != servers_2[server.first])
    {
      const auto it = std::ranges::find_if(
        m_lsp_clients,
        [server](auto const* client)
        {
          return client->lsp_server() == server.first && client->is_running();
        });

      if (it != m_lsp_clients.end())
      {
        (*it)->shutdown();
        m_lsp_clients.erase(it);
      }
      else if (servers_2[server.first])
      {
        lsp_client_add(server.second.second);
      }
    }
  }

  return result;
}

void wex::del::frame::on_command_item_dialog(
  wxWindowID            dialogid,
  const wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_CANCEL:
      if (interruptible::is_running())
      {
        interruptible::end();
        log::status(_("Cancelled"));
      }
      break;

    case wxID_OK:
    case wxID_APPLY:
      switch (dialogid)
      {
        case wxID_ADD:
          if (auto* p = get_project(); p != nullptr)
          {
            data::dir::type_t flags = 0;

            if (config(p->text_addfiles()).get(true))
            {
              flags.set(data::dir::FILES);
            }
            if (config(p->text_addrecursive()).get(true))
            {
              flags.set(data::dir::RECURSIVE);
            }
            if (config(p->text_addfolders()).get(true))
            {
              flags.set(data::dir::DIRS);
            }

            p->add_items(
              config(p->text_infolder()).get_first_of(),
              config(p->text_addwhat()).get_first_of(),
              flags);
          }
          break;

        case id_find_in_files:
        case id_replace_in_files:
          find_in_files((wex::window_id)dialogid);
          break;

        default:
          log::trace("on_command_item_dialog") << event.GetId();
      }
      break;

    default:
      assert(0);
  }
}

wex::syntax::stc* wex::del::frame::stc_entry_dialog_component()
{
  return entry_dialog()->get_stc();
}

int wex::del::frame::stc_entry_dialog_show(bool modal)
{
  return modal ? entry_dialog()->ShowModal() : entry_dialog()->Show();
}

std::string wex::del::frame::stc_entry_dialog_title() const
{
  return m_entry_dialog == nullptr ? std::string() :
                                     m_entry_dialog->GetTitle().ToStdString();
}

void wex::del::frame::stc_entry_dialog_title(const std::string& title)
{
  entry_dialog(title)->SetTitle(title);
}

void wex::del::frame::stc_entry_dialog_validator(const std::string& regex)
{
  entry_dialog()->set_validator(regex);
}
