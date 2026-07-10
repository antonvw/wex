////////////////////////////////////////////////////////////////////////////////
// Name:      json-rpc.cpp
// Purpose:   Implementation of JSON-RPC 2.0 protocol handler
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include <wex/core/log.h>
#include <wex/lsp/json-rpc.h>
#include <wex/lsp/util.h>
#include <wex/ui/defs.h>
#include <wx/event.h>

namespace wex
{
namespace lsp
{

json_rpc::json_rpc(wxEvtHandler* eh)
  : m_event_handler(eh)
{
}

json_rpc_message json_rpc::decode(const std::string& data)
{
  json_rpc_message msg;

  size_t pos = data.find("\r\n\r\n");

  if (pos == std::string::npos)
  {
    log("wex::lsp::json_rpc::decode invalid") << data;
    return msg;
  }

  std::string json_str = data.substr(pos + 4);

  // prevent invalid json
  boost::algorithm::replace_all(json_str, ",\"result\":null", ",\"result\":[]");

  try
  {
    auto parsed = boost::json::parse(json_str);
    auto obj    = parsed.as_object();

    if (obj.contains("jsonrpc"))
    {
      msg.jsonrpc = boost::json::value_to<std::string>(obj["jsonrpc"]);
    }

    if (obj.contains("id"))
    {
      msg.id = boost::json::value_to<int>(obj["id"]);
    }

    if (obj.contains("method"))
    {
      msg.method      = boost::json::value_to<std::string>(obj["method"]);
      msg.is_response = false;
    }
    else if (obj.contains("result") || obj.contains("error"))
    {
      msg.is_response = true;
      msg.is_error    = obj.contains("error");

      if (obj.contains("result"))
      {
        if (obj["result"].is_array())
        {
          msg.result_array = obj["result"].as_array();
        }
        else
        {
          msg.result = obj["result"].as_object();
        }
      }

      if (obj.contains("error"))
      {
        msg.error = obj["error"].as_object();
      }
    }

    if (obj.contains("params"))
    {
      msg.params = obj["params"].as_object();
    }
  }
  catch (std::exception& e)
  {
    log(e) << "wex::lsp::json_rpc::decode:" << json_str;
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

  return make_output(response);
}

std::string json_rpc::encode_notification(
  const std::string&         method,
  const boost::json::object& params)
{
  boost::json::object notification;
  notification["jsonrpc"] = "2.0";
  notification["method"]  = method;
  notification["params"]  = params;

  return make_output(notification);
}

std::string json_rpc::encode_request(
  const std::string&         method,
  const boost::json::object& params)
{
  boost::json::object request;
  request["jsonrpc"] = "2.0";
  request["id"]      = m_id;
  request["method"]  = method;

  if (!params.empty())
  {
    request["params"] = params;
  }

  return make_output(request);
}

std::string json_rpc::encode_response(int id, const boost::json::object& result)
{
  boost::json::object response;
  response["jsonrpc"] = "2.0";
  response["id"]      = id;
  response["result"]  = result;

  return make_output(response);
}

bool json_rpc::handle_response(const json_rpc_message& msg)
{
  if (msg.id > 0)
  {
    // If we have an ID, try to find a handler for it.
    const auto it = m_handlers.find(msg.id);

    if (it == m_handlers.end())
    {
      log("wex::lsp::json_rpc::handle_response")
        << msg.id << "not found size:" << m_handlers.size();
      return false;
    }

    it->second(msg);
    m_handlers.erase(it);

    m_id++;
  }
  else if (msg.method == "textDocument/publishDiagnostics")
  {
    return handle_publish_diagnostics(msg);
  }
  else if (
    msg.method == "window/logMessage" || msg.method == "window/showMessage")
  {
    return handle_show_message(msg);
  }
  else
  {
    log("wex::lsp::json_rpc unhandled") << msg.method;
    return false;
  }

  return true;
}

bool json_rpc::handle_show_message(const json_rpc_message& notification)
{
  auto* msg = new show_message_item(
    notification.params,
    notification.method == "window/showMessage");

  if (!msg->message.empty())
  {
    queue_event(m_event_handler, std::string(), ID_LSP_SHOW_MESSAGE, msg);

    return true;
  }

  return false;
}

std::string json_rpc::header_part_content_field() const
{
  return "Content-Length: ";
}

std::string json_rpc::make_output(const boost::json::object& obj)
{
  std::string json_str = boost::json::serialize(obj);
  std::string header =
    header_part_content_field() + std::to_string(json_str.length());
  std::string output = header + "\r\n\r\n" + json_str;

  return output;
}

void json_rpc::register_handler(response_handler& handler)
{
  m_handlers[m_id] = handler;
}

} // namespace lsp
} // namespace wex
