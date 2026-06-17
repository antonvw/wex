////////////////////////////////////////////////////////////////////////////////
// Name:      listen-to-server.h
// Purpose:   Declaration of class wex::lsp::listen_to_server
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <thread>
#include <stop_token>

#include <wex/lsp/client.h>

namespace wex
{
namespace lsp
{
class client;

/// Offers a class that listens to the Language Server Protocol server.
/// The responses are passed to the client.
class listen_to_server 
{
public:
  /// Contructor, specify client, starts listening.
  listen_to_server(client* cl);

  /// Request to stop listening.
  void request_stop();

private:
  std::string read();
  void run(std::stop_token st);

  client* m_client;

  std::jthread m_worker_thread;
};
}
}
