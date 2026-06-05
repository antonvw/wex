////////////////////////////////////////////////////////////////////////////////
// Name:      json-rpc.cpp
// Purpose:   Implementation of JSON-RPC 2.0 protocol handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/json.hpp>
#include <sstream>

#include <wex/core/log.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
namespace lsp
{
json_rpc_message json_rpc::decode(const std::string& data)
{
  json_rpc_message msg;

  size_t pos = data.find("\r\n\r\n");
  if (pos == std::string::npos)
  {
    return msg;
  }

  std::string json_str = data.substr(pos + 4);

  try
  {
    auto parsed = boost::json::parse(json_str);
    auto obj    = parsed.as_object();

    if (obj.count("jsonrpc"))
    {
      msg.jsonrpc = boost::json::value_to<std::string>(obj["jsonrpc"]);
    }

    if (obj.count("id"))
    {
      msg.id = boost::json::value_to<int>(obj["id"]);
    }

    if (obj.count("method"))
    {
      msg.method      = boost::json::value_to<std::string>(obj["method"]);
      msg.is_response = false;
    }
    else if (obj.count("result") || obj.count("error"))
    {
      msg.is_response = true;
      msg.is_error    = obj.count("error") > 0;

      if (obj.count("result"))
      {
        msg.result = obj["result"].as_object();
      }
      if (obj.count("error"))
      {
        msg.error = obj["error"].as_object();
      }
    }

    if (obj.count("params"))
    {
      msg.params = obj["params"].as_object();
    }
  }
  catch (...)
  {
    return json_rpc_message();
  }

  return msg;
}

std::string json_rpc::encode_error(int id, int code, const std::string& message)
{
  boost::json::object error;
  error["code"]    = code;
  error["message"] = message;

  boost::json::object response;
  response["jsonrpc"] = "2.0";
  response["id"]      = id;
  response["error"]   = error;

  std::string json_str = boost::json::serialize(response);
  std::string header   = "Content-Length: " + std::to_string(json_str.length());
  std::string output   = header + "\r\n\r\n" + json_str;

  return output;
}

std::string json_rpc::encode_notification(
  const std::string&         method,
  const boost::json::object& params)
{
  boost::json::object notification;
  notification["jsonrpc"] = "2.0";
  notification["method"]  = method;

  if (!params.empty())
  {
    notification["params"] = params;
  }

  std::string json_str = boost::json::serialize(notification);
  std::string header   = "Content-Length: " + std::to_string(json_str.length());
  std::string result   = header + "\r\n\r\n" + json_str;

  return result;
}

std::string json_rpc::encode_request(
  const std::string&         method,
  const boost::json::object& params,
  int                        id)
{
  if (id == -1)
  {
    id = next_id();
  }

  boost::json::object request;
  request["jsonrpc"] = "2.0";
  request["id"]      = id;
  request["method"]  = method;

  if (!params.empty())
  {
    request["params"] = params;
  }

  std::string json_str = boost::json::serialize(request);
  std::string header   = "Content-Length: " + std::to_string(json_str.length());
  std::string result   = header + "\r\n\r\n" + json_str;

  return result;
}

std::string json_rpc::encode_response(int id, const boost::json::object& result)
{
  boost::json::object response;
  response["jsonrpc"] = "2.0";
  response["id"]      = id;
  response["result"]  = result;

  std::string json_str = boost::json::serialize(response);
  std::string header   = "Content-Length: " + std::to_string(json_str.length());
  std::string output   = header + "\r\n\r\n" + json_str;

  return output;
}

void json_rpc::handle_response(const json_rpc_message& msg)
{
  auto it = m_handlers.find(msg.id);
  if (it != m_handlers.end())
  {
    it->second(msg);
    m_handlers.erase(it);
  }
  else
  {
    log("json_rpc::handle_response") << msg.id;
  }
}

void json_rpc::register_handler(int id, response_handler handler)
{
  m_handlers[id] = handler;
}

} // namespace lsp
} // namespace wex
