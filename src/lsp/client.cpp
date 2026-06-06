////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/lsp/client.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/item-dialog.h>

namespace wex
{
namespace lsp
{
client::client(const lexer& lexer)
  : m_server_path(lexer.lsp_server())
  , m_language_id(lexer.language())
{
  m_rpc.register_handler(
    1,
    [this](const json_rpc_message& msg)
    {
      if (!msg.is_error && msg.result.count("capabilities"))
      {
        m_capabilities.hover_support      = 1;
        m_capabilities.completion_support = 1;
      }
    });
}

client::~client() = default;

std::vector<std::string>
client::completion(const std::string& uri, int line, int character)
{
  // LSP Methods Implementation
  // Send textDocument/completion request
  // Return list of completion items

  return std::vector<std::string>();
}

int wex::lsp::client::config_dialog(const data::window& par)
{
  const data::window data(
    data::window(par).title(_("Set LSP Server").ToStdString()));

  if (m_item_dialog == nullptr)
  {
    item::choices_bool_t choices;

    for (auto& server : lexers::get()->get_lsp_servers())
    {
      choices.insert(server);
    }

    m_item_dialog = new item_dialog(std::vector<item>{{choices}}, data);
  }
  else
  {
    m_item_dialog->reload();
  }

  return (data.button() & wxAPPLY) ? m_item_dialog->Show() :
                                     m_item_dialog->ShowModal();
}

bool client::did_change(const std::string& uri, const std::string& text)
{
  // LSP Methods Implementation
  // Send textDocument/didChange notification

  return false;
}

bool client::did_open(
  const std::string& uri,
  const std::string& language_id,
  const std::string& text)
{
  // LSP Methods Implementation
  // Send textDocument/didOpen notification

  return false;
}

bool client::did_close(const std::string& uri)
{
  // LSP Methods Implementation
  // Send textDocument/didClose notification

  return false;
}

std::string client::hover(const std::string& uri, int line, int character)
{
  // LSP Methods Implementation
  // Send textDocument/hover request
  // Return hover text

  return std::string();
}

bool client::initialize(const std::string& root_path)
{
  m_context = std::make_unique<boost::asio::io_context>();
  m_process = std::make_unique<boost::process::popen>(
    *m_context,
    boost::process::environment::find_executable(m_server_path),
    std::vector<std::string>{});

  // JSON-RPC Protocol Implementation
  // 1. Send "initialize" request to server
  // 2. Parse server capabilities
  // 3. Send "initialized" notification
  // 4. Set m_initialized = true

  boost::json::object params;
  params["processId"] = nullptr;
  params["rootPath"]  = root_path;

  if (!write(m_rpc.encode_request("initialize", params)))
  {
    return false;
  }

  const auto& response_rpc(m_rpc.decode(read()));

  if (response_rpc.id == -1)
  {
    return false;
  }

  if (!m_rpc.handle_response(response_rpc))
  {
    return false;
  }

  if (!write(m_rpc.encode_notification("initialized")))
  {
    return false;
  }

  m_initialized = true;

  log::debug("lsp init") << m_server_path;

  return m_initialized;
}

bool client::is_running() const
{
  return m_process && m_process->running();
}

std::string client::read()
{
  std::string data, response;
  size_t      n, max = UINT_MAX, total = 0;

  while ((n = boost::asio::read_until(
            *m_process,
            boost::asio::dynamic_buffer(data),
            "}")) > 0)
  {
    response += data.substr(0, n);
    total += n;
    data.erase(0, n);

    if (max == UINT_MAX)
    {
      const size_t header_length = 24;

      if (regex r("Content-Length: ([0-9]+).*"); r.match(response) > 0)
      {
        max = stoi(r[0]) + header_length;
      }
    }

    if (total == max)
    {
      break;
    }
  }

  return response;
}

bool client::shutdown()
{
  // JSON-RPC Protocol Implementation
  // 1. Send "shutdown" request
  // 2. Send "exit" notification
  // 3. Stop process

  if (
    !m_initialized || !write(m_rpc.encode_request("shutdown")) ||
    !write(m_rpc.encode_notification("exit")))
  {
    return false;
  }

  m_process->terminate();
  m_initialized = false;

  log::debug("lsp shutdown") << m_server_path;

  return true;
}

bool client::write(const std::string& text)
{
  if (m_process == nullptr)
  {
    return false;
  }

  return boost::asio::write(*m_process, boost::asio::buffer(text)) > 0;
}
} // namespace lsp
} // namespace wex
