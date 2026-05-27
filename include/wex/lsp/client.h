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

#include <wex/factory/process.h>
#include <wex/lsp/json-rpc.h>

namespace wex
{
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
class client
{
public:
  /// Constructor.
  /// \param server_path Path to the language server executable
  /// \param language_id Language identifier (e.g., "cpp", "python")
  client(const std::string& server_path, const std::string& language_id);

  /// Destructor.
  ~client();

  /// Initializes the LSP client connection.
  /// Returns true if successfully initialized.
  bool initialize();

  /// Shuts down the LSP connection gracefully.
  /// Returns true if shutdown was successful.
  bool shutdown();

  /// Returns server capabilities.
  const capabilities& get_capabilities() const { return m_capabilities; }

  /// Returns whether the client is initialized.
  bool is_initialized() const { return m_initialized; }

  /// Returns whether the client is running.
  bool is_running() const { return m_process && m_process->is_running(); }

  /// Opens a document for editing.
  /// Returns true if successful.
  bool did_open(
    const std::string& uri,
    const std::string& language_id,
    const std::string& text);

  /// Notifies server of document changes.
  /// Returns true if successful.
  bool did_change(const std::string& uri, const std::string& text);

  /// Closes a document.
  /// Returns true if successful.
  bool did_close(const std::string& uri);

  /// Requests hover information.
  /// Returns hover text if available, empty string otherwise.
  std::string hover(const std::string& uri, int line, int character);

  /// Requests completion at position.
  /// Returns list of completion items.
  std::vector<std::string> completion(
    const std::string& uri,
    int                line,
    int                character);

private:
  std::string                      m_server_path;
  std::string                      m_language_id;
  std::shared_ptr<factory::process> m_process;
  capabilities                     m_capabilities;
  bool                             m_initialized{false};
  json_rpc                         m_rpc;
};

} // namespace lsp
} // namespace wex
