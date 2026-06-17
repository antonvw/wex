////////////////////////////////////////////////////////////////////////////////
// Name:      client.h
// Purpose:   Declaration of class wex::lsp::client
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/process.hpp>

#include <wex/core/path.h>
#include <wex/factory/window.h>
#include <wex/lsp/json-rpc.h>
#include <wex/lsp/listen-to-server.h>
#include <wex/syntax/lexer.h>
#include <wex/ui/lsp.h>

class wxEvtHandler;

namespace wex
{
class item_dialog;

namespace lsp
{
/// Server capabilities tracking.
struct capabilities
{
  bool hover_support{false};
  bool completion_support{false};
  bool definition_support{false};
  bool references_support{false};
  bool rename_support{false};
  bool formatting_support{false};
  bool diagnostic_support{false};
};

/// Represents the Language Server Protocol client.
/// Each client communicates with one Server, based upon lexer setup.
class client
{
  friend class listen_to_server;
public:
  /// Shows a dialog allowing you to choose which lsp server to use
  /// Returns dialog return code.
  static int config_dialog(const data::window& data = data::window());

  /// Constructor, specify lexer for which to create an LSP client, and handler
  /// for UI updates. The lexer determines which LSP server to use based on its
  /// configuration. If event_handler is nullptr, the client will not post
  /// events for UI updates.
  client(const lexer& lexer, wxEvtHandler* event_handler = nullptr);

  /// Destructor.
  ~client();

  /// Requests code completion at position.
  /// Returns true if successful.
  bool completion(const wex::path& path, const position_item& pos);

  /// Requests goto definition information.
  /// Returns true if successful.
  bool definition(const wex::path& path, const position_item& pos);

  /// Notifies server of document changes.
  /// Returns true if successful.
  bool did_change(const wex::path& path, const std::string& text);

  /// Notifies server of closed document.
  /// Returns true if successful.
  bool did_close(const wex::path& path);

  /// Notifies server of opened document.
  /// Returns true if successful.
  bool did_open(const wex::path& path, const std::string& text);

  /// Returns extensions.
  const std::string& extensions() const { return m_extensions; }

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
  const std::string& language_id() const { return m_language_id; }

  /// Shuts down the LSP connection gracefully.
  /// Returns true if shutdown was successful.
  bool shutdown();

private:
  bool        definition_or_implementation(
    const wex::path& path, const position_item& pos, const std::string& method);
  bool        write(const std::string& text, response_handler resp = nullptr);

  wxEvtHandler* m_event_handler{nullptr};

  std::string m_extensions, m_language_id, m_server_flags, m_server_path;

  std::unique_ptr<boost::asio::io_context> m_context;
  std::unique_ptr<boost::process::popen>   m_process;
  std::unique_ptr<listen_to_server>        m_listen_to_server;

  capabilities m_capabilities;
  bool         m_initialized{false};
  json_rpc     m_rpc;

  static inline item_dialog* m_item_dialog{nullptr};
};

} // namespace lsp
} // namespace wex
