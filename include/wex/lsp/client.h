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

#include <boost/process.hpp>

#include <wex/lsp/json-rpc.h>
#include <wex/syntax/lexer.h>

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
/// Each client communicates with one Server, based upon lexer setup.
class client
{
public:
  /// Constructor.
  /// \param lexer The lexer for which to create an LSP client
  client(const lexer& lexer);

  /// Destructor.
  ~client();

  /// Requests completion at position.
  /// Returns list of completion items.
  std::vector<std::string>
  completion(const std::string& uri, int line, int character);

  /// Notifies server of document changes.
  /// Returns true if successful.
  bool did_change(const std::string& uri, const std::string& text);

  /// Closes a document.
  /// Returns true if successful.
  bool did_close(const std::string& uri);

  /// Opens a document for editing.
  /// Returns true if successful.
  bool did_open(
    const std::string& uri,
    const std::string& language_id,
    const std::string& text);

  /// Returns server capabilities.
  const capabilities& get_capabilities() const { return m_capabilities; }

  /// Initializes the LSP client connection.
  /// Returns true if successfully initialized.
  bool initialize();

  /// Returns whether the client is initialized.
  bool is_initialized() const { return m_initialized; }

  /// Returns whether the client is running.
  bool is_running() const;

  /// Requests hover information.
  /// Returns hover text if available, empty string otherwise.
  std::string hover(const std::string& uri, int line, int character);

  /// Shuts down the LSP connection gracefully.
  /// Returns true if shutdown was successful.
  bool shutdown();

private:
  bool write(const std::string& text);

  std::string m_server_path;
  std::string m_language_id;

  std::shared_ptr<boost::process::popen> m_process;

  capabilities m_capabilities;
  bool         m_initialized{false};
  json_rpc     m_rpc;
};

} // namespace lsp
} // namespace wex
