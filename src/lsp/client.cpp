////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/client.h>

namespace wex
{
namespace lsp
{
client::client(const std::string& server_path, const std::string& language_id)
  : m_server_path(server_path)
  , m_language_id(language_id)
{
  m_process = std::make_shared<wex::factory::process>();
  m_process->set_handler_out(this);
  m_process->async_system("clangd --path-mappings=" + server_path);
}

client::~client() = default;

bool client::initialize()
{
  // JSON-RPC Protocol Implementation
  // 1. Send "initialize" request to server
  // 2. Parse server capabilities
  // 3. Send "initialized" notification
  // 4. Set m_initialized = true

  boost::json::object params;
  params["processId"] = nullptr;
  params["rootPath"]  = "/home/user/project";

  wex::lsp::json_rpc rpc;

  if (
    m_process->write(rpc.encode_request("initialize", params)) &&
    m_process->write(rpc.encode_notification("initialized")))
  {
    m_initialized = true;
  }

  return m_initialized;
}

bool client::shutdown()
{
  // JSON-RPC Protocol Implementation
  // 1. Send "shutdown" request
  // 2. Send "exit" notification
  // 3. Stop process

  wex::lsp::json_rpc rpc;

  return m_initialized && m_process->write(rpc.encode_request("shutdown")) &&
         m_process->write(rpc.encode_notification("exit")) && m_process->stop();
}

bool client::did_open(
  const std::string& uri,
  const std::string& language_id,
  const std::string& text)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/didOpen notification

  return false;
}

bool client::did_change(const std::string& uri, const std::string& text)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/didChange notification

  return false;
}

bool client::did_close(const std::string& uri)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/didClose notification

  return false;
}

std::string client::hover(const std::string& uri, int line, int character)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/hover request
  // Return hover text

  return std::string();
}

std::vector<std::string>
client::completion(const std::string& uri, int line, int character)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/completion request
  // Return list of completion items

  return std::vector<std::string>();
}

} // namespace lsp
} // namespace wex
