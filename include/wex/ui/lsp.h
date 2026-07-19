////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.h
// Purpose:   Declaration of classes related to Language Server Protocol (LSP)
//            support in wex.
//            Uses LSP Specification - 3.17.0
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <boost/json.hpp>
#include <wx/stc/stc.h>

namespace wex
{
/// Diagnostic severity levels.
enum class severity_t
{
  ERRORS  = 1,
  WARNING = 2,
  INFO    = 3,
  HINT    = 4
};

/// - Each class used in a request will have a json_object() function to convert
///   it to a JSON object for communication with the language server.
/// - Each class used in a response will have a constructor that takes a JSON
/// object
///   to initialize the class from the server's response.

/// Represents a position in a document, including line and character offsets.
struct position_item
{
  /// Default constructor.
  position_item(int l = 0, int c = 0);

  /// Constructs a position_item from a wxStyledTextCtrl.
  position_item(wxStyledTextCtrl* stc);

  /// Returns a JSON object representation of the position_item.
  boost::json::object json_object() const;

  /// Logs info about this class.
  std::stringstream log() const;

  int line{0};      // Line position in a document (0-based)
  int character{0}; // Character offset on a line in a document (0-based)
};

/// Represents a range in a document, defined by two positions.
struct range_item
{
  /// Returns a JSON object representation of the range_item.
  boost::json::object json_object() const;

  /// Logs info about this class.
  std::stringstream log() const;

  position_item start, end;
};

/// Represents an element of a completion item.
struct completion_item_element
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  completion_item_element(const boost::json::object& obj);

  std::string
    label; // Label of the completion item (e.g., function name, variable name)
  int         kind{0}; // Completion item kind (e.g., function, variable, etc.)
  std::string detail;  // Additional details about the completion item
  std::string documentation; // Documentation string for the completion item
};

/// Represents a completion item.
struct completion_item
{
  position_item pos;

  std::vector<completion_item_element> elements;
};

/// Represents a definition or implementation item.
struct definition_or_implementation_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  definition_or_implementation_item(const boost::json::object& obj);

  std::string uri;

  range_item range;
};

/// Represents a single diagnostic item(error, warning, etc.).
struct diagnostic_item
{
  /// Default contrusctor.
  diagnostic_item() { ; };

  /// Constructor from a JSON object,
  /// as received from the language server.
  diagnostic_item(const boost::json::object& obj);

  range_item range;

  /// Severity of the diagnostic
  severity_t severity{severity_t::INFO};

  /// Diagnostic code
  std::string code;

  /// Diagnostic message
  std::string message;

  /// Source of the diagnostic (e.g., "clang", "gcc")
  std::string source;
};

/// Represents hover information.
struct hover_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  hover_item(const boost::json::object& obj);

  position_item pos;

  /// Contents of the hover information
  std::string contents;

  /// Kind of the hover information, like markdown.
  std::string kind;
};

struct on_type_formatting_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  on_type_formatting_item(const boost::json::object& obj);

  std::string new_text;

  range_item range;
};

/// Represents a show or a log message item.
struct show_message_item
{
  /// Message type values.
  enum message_t
  {
    ERRORS = 1,
    WARNING,
    INFO,
    LOG,
    DEBUG // @since 3.18.0
  };

  /// Constructor from a JSON object,
  /// as received from the language server.
  show_message_item(const boost::json::object& obj, bool is_show_item = true);

  message_t   type;
  std::string message;
  const bool  is_show{true};
};

/// Convert a json value that has a key with a string value to a string.
/// Returns true if value can be converted.
bool json_to_string(
  /// the value
  const boost::json::value& val,
  /// the key
  const std::string& key,
  /// the text containing conversion
  std::string& text);

/// Convert a json range object to a range item.
/// Returns true if value can be converted.
bool range_from_json(
  /// the value
  const boost::json::object& obj,
  /// the range containing conversion
  range_item& range);

/// Type alias for collections of completions returned by the language server.
using completions_t = completion_item;

/// Type alias for a definition or implementation item returned by the language
/// server.
using definition_or_implementation_t =
  std::vector<definition_or_implementation_item>;

/// Type alias for a collection of diagnostics returned by the language server.
using diagnostics_t = std::vector<diagnostic_item>;

/// Type alias for a hover item returned by the language server.
using hover_t = struct hover_item;

/// Type alias for on type format.
using on_type_formatting_item_t = std::vector<on_type_formatting_item>;
} // namespace wex
