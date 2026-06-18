////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/lsp/client.h>
#include <wex/lsp/util.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/defs.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/lsp.h>

namespace wex
{
boost::json::object make_object(const wex::path& path, const position_item& p)
{
  boost::json::object params, text_doc, pos;

  text_doc["uri"] = path.uri();

  pos["line"]      = p.line;
  pos["character"] = p.character;

  params["position"]     = pos;
  params["textDocument"] = text_doc;

  return params;
}

namespace lsp
{
client::client(const lexer& lexer, wxEvtHandler* event_handler)
  : m_server_path(lexer.lsp_server())
  , m_server_flags(lexer.lsp_server_flags())
  , m_language_id(lexer.scintilla_lexer())
  , m_rpc(event_handler)
  , m_event_handler(event_handler)
  , m_extensions(lexer.extensions())
{
}

bool client::completion(
  const wex::path& path,
  const position_item& pos,
  const std::string& trigger,
  bool is_incomplete)
{
  // LSP Methods Implementation
  // send textDocument/completion request
  auto obj(make_object(path, pos));
  boost::json::object context;

  if (!trigger.empty())
  {
    context["triggerCharacter"] = trigger;
    context["triggerKind"] = 2;
  }
  else
  {
    context["triggerKind"] = (is_incomplete ? 3 : 1);
  }

  obj["context"] = context;

  if (!write(
        m_rpc.encode_request("textDocument/completion", obj),
        [=, this](const json_rpc_message& msg)
        {
          if (!msg.is_error && msg.result.contains("items"))
          {
            completions_t* completion = new completions_t;
            completion->pos           = pos;
            completion->elements.reserve(
              msg.result.at("items").as_array().size());

            for (const auto& item : msg.result.at("items").as_array())
            {
              completion_item_element completion_element;

              if (item.as_object().contains("label"))
              {
                completion_element.label =
                  item.as_object().at("label").as_string().data();
              }

              if (item.as_object().contains("kind"))
              {
                completion_element.kind =
                  item.as_object().at("kind").as_int64();
              }

              if (item.as_object().contains("detail"))
              {
                completion_element.detail =
                  item.as_object().at("detail").as_string().data();
              }

              if (item.as_object().contains("documentation"))
              {
                // The documentation is an array, not yet handled
                // completion_element.documentation =
              }

              completion->elements.push_back(completion_element);
            }

            queue_event(
              m_event_handler,
              path.uri(),
              ID_LSP_CODE_COMPLETION,
              completion);
          }
        }))
  {
    return false;
  }

  return true;
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
      choices.insert(server.first);
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

bool client::definition(const wex::path& path, const position_item& pos)
{
  return definition_or_implementation(path, pos, "textDocument/definition");
}

bool client::definition_or_implementation(
  const wex::path& path, const position_item& pos, const std::string& method)
{
  if (!write(
        m_rpc.encode_request(method, make_object(path, pos)),
        [=, this](const json_rpc_message& msg)
        {
          if (!msg.is_error)
          {
            auto* definition = new definition_or_implementation_t;

            for (const auto& item : msg.result_array)
            {
              definition_or_implementation_item di;
              range_from_json(item.as_object(), di.range);
              di.uri = item.as_object().at("uri").as_string().data();

              definition->push_back(di);
            }

            if (definition->empty())
            {
              delete definition;
              definition = nullptr;
            }

            queue_event(
              m_event_handler,
              path.uri(),
              method == "textDocument/definition" ? 
                ID_LSP_DEFINITION : ID_LSP_IMPLEMENTATION,
              definition);
          }
        }))
  {
    return false;
  }

  return true;
}

bool client::did_change(const wex::path& path, const std::string& text)
{
  // LSP Methods Implementation
  // send textDocument/didChange notification
  boost::json::object params, text_doc;
  text_doc["uri"]        = path.uri();
  text_doc["text"]       = text;
  params["textDocument"] = text_doc;

  return write(m_rpc.encode_notification("textDocument/didChange", params));
}

bool client::did_close(const wex::path& path)
{
  // LSP Methods Implementation
  // send textDocument/didClose notification
  boost::json::object params, text_doc;
  text_doc["uri"]        = path.uri();
  params["textDocument"] = text_doc;

  return write(m_rpc.encode_notification("textDocument/didClose", params));
}

bool client::did_open(const wex::path& path, const std::string& text)
{
  // LSP Methods Implementation
  // send textDocument/didOpen notification
  boost::json::object params, text_doc;
  text_doc["uri"]        = path.uri();
  text_doc["languageId"] = m_language_id;
  text_doc["text"]       = text;
  params["textDocument"] = text_doc;

  return write(m_rpc.encode_notification("textDocument/didOpen", params));
}

bool client::hover(const wex::path& path, const position_item& pos)
{
  // LSP Methods Implementation
  // send textDocument/hover request
  if (!write(
        m_rpc.encode_request("textDocument/hover", make_object(path, pos)),
        [=, this](const json_rpc_message& msg)
        {
          if (!msg.is_error && msg.result.contains("result"))
          {
            auto* hover      = new hover_t;
            hover->contents  = boost::json::serialize(msg.result);
            hover->pos       = pos;

            queue_event(m_event_handler, path.uri(), ID_LSP_HOVER, hover);
          }
        }))
  {
    return false;
  }

  return true;
}

bool client::implementation(const wex::path& path, const position_item& pos)
{
  return definition_or_implementation(path, pos, "textDocument/implementation");
}

bool client::initialize(const wex::path& root_path)
{
  try
  {
    const std::vector<std::string> flags{{"--" + m_server_flags}};
    m_context = std::make_unique<boost::asio::io_context>();
    m_process = std::make_unique<boost::process::popen>(
      *m_context,
      boost::process::environment::find_executable(m_server_path),
      flags);
  }
  catch (std::exception& e)
  {
    log(e) << "wex::lsp::client::initialize with path" << m_server_path;
    return false;
  }

  // JSON-RPC Protocol Implementation
  // 1. Send "initialize" request to server
  // 2. Parse server capabilities
  // 3. Send "initialized" notification
  // 4. Set m_initialized = true

  boost::json::object params;
  params["processId"] = nullptr;
  params["rootPath"]  = root_path.string();

  if (!write(
        m_rpc.encode_request("initialize", params),
        [this](const json_rpc_message& msg)
        {
          if (!msg.is_error && msg.result.contains("capabilities"))
          {
            m_capabilities.hover_support      = 1;
            m_capabilities.completion_support = 1;
          }
        }))
  {
    return false;
  }

  if (!write(m_rpc.encode_notification("initialized")))
  {
    return false;
  }

  m_initialized = true;

  log::debug("lsp init") << m_server_path;

  m_listen_to_server = std::make_unique<listen_to_server>(this);

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
    !m_initialized ||
    !write(
      m_rpc.encode_request("shutdown"),
      [this](const json_rpc_message& msg)
      {
        if (msg.is_error)
        {
          log("shutdown failed") << msg.id;
        }
      }) ||
    !write(m_rpc.encode_notification("exit")))
  {
    return false;
  }

  m_listen_to_server->request_stop();
  m_process->terminate();
  m_initialized = false;

  log::debug("lsp shutdown") << m_server_path;

  return true;
}

bool client::write(const std::string& text, response_handler resp)
{
  if (
    m_process == nullptr || !m_process->running() ||
    boost::asio::write(*m_process, boost::asio::buffer(text)) == 0)
  {
    return false;
  }

  if (resp != nullptr)
  {
    m_rpc.register_handler(resp);
  }

  return true;
}
} // namespace lsp
} // namespace wex
