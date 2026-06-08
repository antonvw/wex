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
#include <wex/lsp/notification-handler.h>
#include <wex/syntax/lexer.h>

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

/// Represents a Language Server Protocol client.
/// Each client communicates with one Server, based upon lexer setup.
class client
{
  friend notification_handler;

public:
  /// Shows a dialog allowing you to choose which lsp server to use
  /// Returns dialog return code.
  static int config_dialog(const data::window& data = data::window());

  /// Constructor, specify lexer for which to create an LSP client
  client(const lexer& lexer);

  /// Destructor.
  ~client();

  /// Requests completion at position.
  /// Returns list of completion items.
  std::vector<std::string>
  completion(const wex::path& path, int line, int character);

  /// Requests definition information.
  /// Returns location text if available, empty string otherwise.
  std::string definition(const wex::path& path, int line, int character);

  /// Closes a document.
  /// Returns true if successful.
  bool did_close(const wex::path& path);

  /// Notifies server of document changes.
  /// Returns true if successful.
  bool did_change(const wex::path& path, const std::string& text);

  /// Opens a document for editing.
  /// Returns true if successful.
  bool did_open(const wex::path& path, const std::string& text);

  /// Returns server capabilities.
  const capabilities& get_capabilities() const { return m_capabilities; }

  /// Requests hover information.
  /// Returns hover text if available, empty string otherwise.
  std::string hover(const wex::path& path, int line, int character);

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
  std::string definition_or_hover(
    const wex::path&   path,
    int                line,
    int                character,
    const std::string& which);

  std::string read();
  bool        write(const std::string& text, response_handler resp = nullptr);

  std::string m_language_id, m_server_flags, m_server_path;

  std::unique_ptr<boost::asio::io_context> m_context;
  std::unique_ptr<boost::process::popen>   m_process;
  std::unique_ptr<notification_handler>    m_notification_handler;

  capabilities m_capabilities;
  bool         m_initialized{false};
  json_rpc     m_rpc;

  static inline item_dialog* m_item_dialog{nullptr};
};

} // namespace lsp
} // namespace wex
