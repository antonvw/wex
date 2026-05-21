////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/client.h>

namespace wex::lsp
{
client::client(const std::string& server_path, const std::string& language_id)
  : m_server_path(server_path)
  , m_language_id(language_id)
{
}

client::~client()
{
  if (m_initialized)
  {
    shutdown();
  }
}

bool client::initialize()
{
  // TODO: Implement JSON-RPC initialization message
  // Send: {"jsonrpc": "2.0", "id": 1, "method": "initialize",
  //        "params": {"processId": pid, "rootPath": "", ...}}
  // Receive capabilities from server
  
  m_initialized = true;
  return true;
}

bool client::shutdown()
{
  // TODO: Send shutdown request to server
  // Send: {"jsonrpc": "2.0", "id": X, "method": "shutdown", "params": {}}
  
  if (m_process)
  {
    m_process->stop();
  }
  
  m_initialized = false;
  return true;
}

bool client::did_open(
  const std::string& uri,
  const std::string& language_id,
  const std::string& text)
{
  // TODO: Send textDocument/didOpen notification
  // Send: {"jsonrpc": "2.0", "method": "textDocument/didOpen",
  //        "params": {"textDocument": {"uri": uri, "languageId": language_id,
  //                                     "version": 1, "text": text}}}
  
  return true;
}

bool client::did_change(const std::string& uri, const std::string& text)
{
  // TODO: Send textDocument/didChange notification
  // Send: {"jsonrpc": "2.0", "method": "textDocument/didChange",
  //        "params": {"textDocument": {"uri": uri, "version": N},
  //                   "contentChanges": [{"text": text}]}}
  
  return true;
}

bool client::did_close(const std::string& uri)
{
  // TODO: Send textDocument/didClose notification
  
  return true;
}

std::string client::hover(
  const std::string& uri,
  int                line,
  int                character)
{
  // TODO: Send textDocument/hover request
  // Receive: {"jsonrpc": "2.0", "id": X, "result": {"contents": "..."}}
  
  return std::string();
}

std::vector<std::string> client::completion(
  const std::string& uri,
  int                line,
  int                character)
{
  // TODO: Send textDocument/completion request
  // Receive list of completion items
  
  return std::vector<std::string>();
}

} // namespace wex::lsp
