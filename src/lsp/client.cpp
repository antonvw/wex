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
  // TODO: Initialize process with server_path
}

client::~client() = default;

bool client::initialize()
{
  // TODO: Phase 1 - JSON-RPC Protocol Implementation
  // 1. Send "initialize" request to server
  // 2. Parse server capabilities
  // 3. Send "initialized" notification
  // 4. Set m_initialized = true
  
  return false;
}

bool client::shutdown()
{
  // TODO: Phase 1 - JSON-RPC Protocol Implementation
  // 1. Send "shutdown" request
  // 2. Send "exit" notification
  // 3. Stop process
  
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

std::vector<std::string> client::completion(
  const std::string& uri,
  int                line,
  int                character)
{
  // TODO: Phase 2 - LSP Methods Implementation
  // Send textDocument/completion request
  // Return list of completion items
  
  return std::vector<std::string>();
}

} // namespace lsp
} // namespace wex
