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

void client::listen_to_server()
{
  std::thread t(
    [this]
    {
      while (is_running())
      {
        try
        {
          if (const auto& response = read(); !response.empty())
          {
            if (
              const auto& response_rpc(m_rpc.decode(response));
              response_rpc.id != -1)
            {
              m_rpc.handle_response(response_rpc);
            }
          }
        }
        catch (const std::exception& e)
        {
          log(e) << "listen_to_server";
        }
      }
    });

  t.detach();
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
      if (
        regex r(m_rpc.header_part_content_field() + "([0-9]+).*");
        r.match(response) > 0)
      {
        const size_t header_length =
          m_rpc.header_part_content_field().size() + r[0].size() + 4;

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

} // namespace lsp
} // namespace wex
