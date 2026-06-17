////////////////////////////////////////////////////////////////////////////////
// Name:      listen-to-server.cpp
// Purpose:   Implementation of client::listen_to_server
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/lsp/client.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
namespace lsp
{

listen_to_server::listen_to_sever(client* cl)
  : m_client(cl)
  , m_worker_thread(&listen_to_server::run)
{
}

std::string listen_to_server::read()
{
  std::string               data, response;
  size_t                    n, max = UINT_MAX, total = 0;
  boost::system::error_code ec;

  while ((n = boost::asio::read_until(
            m_client->m_process,
            boost::asio::dynamic_buffer(data),
            "}",
            ec)) > 0 ||
         !data.size())
  {
    if (ec == boost::asio::error::eof)
    {
      return std::string();
    }

    response += data.substr(0, n);
    total += n;
    data.erase(0, n);

    if (max == UINT_MAX)
    {
      if (
        regex r(json_rpc().header_part_content_field() + "([0-9]+).*");
        r.match(response) > 0)
      {
        const size_t header_length =
          json_rpc().header_part_content_field().size() + r[0].size() + 4;

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

void listen_to_server::request_stop()
{
  m_worker_thread.request_stop();
}

void listen_to_server::run(std::stop_token st)
{
  while ((!st.stop_requested() && m_client->is_running())
  {
    try
    {
      if (const auto& response = read(); !response.empty())
      {
        const auto& response_rpc(m_client->m_rpc.decode(response));
        m_client->m_rpc.handle_response(response_rpc);
      }
      else
      {
        break;
      }
    }
    catch (const std::exception& e)
    {
      log(e) << "wex::lsp::listen_to_server::run";
    }
  }
}

} // namespace lsp
} // namespace wex
