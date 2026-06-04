////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/asio.hpp>
#include <wex/lsp/client.h>

namespace wex
{
namespace lsp
{
client::client(const lexer& lexer)
  : m_server_path(lexer.lsp())
  , m_language_id(lexer.language())
{
  boost::asio::io_context ctx;

  m_process = std::make_shared<boost::process::popen>(
    ctx,
    m_server_path, 
    std::vector<std::string{"--path-mappings=" + server_path});

  m_rpc.register_handler(1, [this](const json_rpc_message& msg) 
  {
    if (!msg.is_error && msg.result.count("capabilities")) 
    {
        const auto caps = msg.result["capabilities"];
        m_capabilities.hover_support = 1;
        m_capabilities.completion_support = 1;
    }
  });
}

client::~client() = default;

std::vector<std::string>
client::completion(const std::string& uri, int line, int character)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/completion request
  // Return list of completion items

  return std::vector<std::string>();
}

bool client::did_change(const std::string& uri, const std::string& text)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/didChange notification

  return false;
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

  if (!write(m_rpc.encode_request("initialize", params)))
  {
    return false;
  }

  std::string response;
  boost::asio::read_until(*m_process, boost::asio::dynamic_buffer(response), '\n');

  m_rpc.handle_response(m_rpc.decode(response));

  if (!write(m_rpc.encode_notification("initialized")))
  {
    return false;
  }

  m_initialized = true;

  return m_initialized;
}

bool client::is_running() const
{
  return m_process && m_process->running();
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

  return true;
}

bool client::write(const std::string& text)
{
  if (m_process == nullptr)
  {
    return false;
  }
  
  boost::asio::write(*m_process, boost::asio::buffer(text));
}
} // namespace lsp
} // namespace wex
