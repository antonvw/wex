////////////////////////////////////////////////////////////////////////////////
// Name:      client.h
// Purpose:   Declaration of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <wex/core/path.h>
#include <wex/factory/window.h>
#include <wex/lsp/capabilities.h>
#include <wex/lsp/json-rpc.h>
#include <wex/lsp/listen-to-server.h>
#include <wex/syntax/lexer.h>
#include <wex/ui/lsp.h>

class wxEvtHandler;

namespace wex
{
namespace lsp
{
/// Represents the Language Server Protocol client.
/// Each client communicates with one Server, based upon lexer setup.
class client
{
  friend class listen_to_server;

public:
  /// Constructor, specify lexer for which to create a LSP client, and handler
  /// for UI updates. The lexer determines which LSP server to use based on its
  /// configuration. If event_handler is nullptr, the client will not post
  /// events for UI updates.
  client(lexer lexer, wxEvtHandler* event_handler = nullptr);

  /// Destructor.
  ~client() = default;

  /// Requests code completion at position.
  /// Returns true if successful.
  bool completion(
    /// the path
    const wex::path& path,
    /// the position
    const position_item& pos,
    /// the trigger, such as:
    /// - "."  object memmbers
    /// - "::" class members
    /// - "->" pinter members
    const std::string& trigger = std::string(),
    /// previous completion list was marked isIncomplete: true
    /// this lets the server refine suggestions as the user keeps typing
    bool is_incomplete = false);

  /// Requests goto definition information.
  /// Returns true if successful.
  bool definition(const wex::path& path, const position_item& pos);

  /// Notifies server of document changes.
  /// Returns true if successful.
  bool did_change(
    const wex::path&   path,
    const range_item&  range,
    const std::string& text);

  /// Notifies server of closed document.
  /// Returns true if successful.
  bool did_close(const wex::path& path);

  /// Notifies server of opened document.
  /// Returns true if successful.
  bool did_open(const wex::path& path, const std::string& text);

  /// Notifies server of saved document.
  /// Returns true if successful.
  bool did_save(const wex::path& path);

  /// Returns extensions.
  const std::string& extensions() const;

  /// Returns server capabilities.
  const capabilities& get_capabilities() const { return m_capabilities; }

  /// Requests hover (tooltip) information.
  /// Returns true if successful.
  bool hover(const wex::path& path, const position_item& pos);

  /// Requests goto implementation information.
  /// Returns true if successful.
  bool implementation(const wex::path& path, const position_item& pos);

  /// Initializes the LSP client connection with root path.
  /// Returns true if successfully initialized.
  bool initialize(const wex::path& root_path);

  /// Returns whether the client is initialized.
  bool is_initialized() const { return m_initialized; }

  /// Returns whether the client is running.
  bool is_running() const;

  /// Returns language id.
  const std::string& language_id() const;

  /// Returns the lsp server.
  const std::string& lsp_server() const;

  /// Requests on type formatting information.
  /// Returns true if successful.
  bool on_type_formatting(
    const wex::path&     path,
    const position_item& pos,
    char                 c,
    bool                 use_tabs,
    int                  tab_size);

  /// Shuts down the LSP connection gracefully.
  /// Returns true if shutdown was successful.
  bool shutdown();

  /// Returns version for specified uri.
  int version(const std::string& uri) const;

private:
  bool definition_or_implementation(
    const wex::path&     path,
    const position_item& pos,
    const std::string&   method);
  bool initialize_prepare();
  bool write(const std::string& text, response_handler resp = nullptr);

  wxEvtHandler* m_event_handler{nullptr};

  std::unique_ptr<boost::asio::io_context> m_context;
  std::unique_ptr<boost::process::popen>   m_process;
  std::unique_ptr<listen_to_server>        m_listen_to_server;

  capabilities m_capabilities;
  bool         m_initialized{false};
  json_rpc     m_rpc;

  const lexer m_lexer;

  std::unordered_map<std::string, int> m_uri_versions;
};

} // namespace lsp
} // namespace wex
