////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.h
// Purpose:   Declaration of classes related to Language Server Protocol (LSP)
//            support in wex.
//            Uses LSP Specification - 3.17.0
//            https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
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
  ERRORS  = 1, ///< errors level
  WARNING = 2, ///< warning level
  INFO    = 3, ///< info level
  HINT    = 4  ///< hint level
};

/// Represents a JSON-RPC 2.0 message.
struct json_rpc_message
{
  /// Request ID; -1 for notifications.
  int id{-1};

  /// Method name.
  std::string method;

  boost::json::object error, ///< error object
    params,                  ///< request parameters
    result;                  ///< result object

  /// Array-form result.
  boost::json::array result_array;

  /// True when this message represents an error.
  bool is_error{false};
};

// - Each class used in a request will have a json_object() function to convert
//   it to a JSON object for communication with the language server.
// - Each class used in a response will have a constructor that takes a JSON
//   object to initialize the class from the server's response.

/// Represents a position in a document, including line and character offsets.
struct position_item
{
  /// Default constructor.
  position_item(int l = 0, int c = 0);

  /// Constructs a position_item from current position on a wxStyledTextCtrl.
  position_item(wxStyledTextCtrl* stc);

  /// Constructor from a json object.
  position_item(const boost::json::object& obj);

  /// Returns a JSON object representation of the position_item.
  boost::json::object json_object() const;

  /// Logs info about this class.
  std::stringstream log() const;

  /// Converts the position to a text position in the given control.
  int to_pos(wxStyledTextCtrl* stc) const;

  int line{0},    ///< line position in a document (0-based)
    character{0}; ///< character offset on a line in a document (0-based)
};

/// Represents a range in a document, defined by two positions.
struct range_item
{
  /// Default constructor.
  range_item(
    const position_item& strt = position_item(),
    const position_item& end  = position_item());

  /// Constructor from a json object.
  range_item(const boost::json::object& obj);

  /// Returns a JSON object representation of the range_item.
  boost::json::object json_object() const;

  /// Logs info about this class.
  std::stringstream log() const;

  /// Sets from a json range object item.
  /// Returns true if value can be converted.
  bool set(const boost::json::object& obj);

  /// Sets the target range in the given wxStyledTextCtrl based on this
  /// range_item.
  void set_target(wxStyledTextCtrl* stc) const
  {
    stc->SetTargetStart(start.to_pos(stc));
    stc->SetTargetEnd(end.to_pos(stc));
  }

  position_item start, ///< start of range
    end;               ///< end of range
};

/// Represents a single code action edit change.
struct code_action_edit_change_item
{
  /// Default constructor, taking a range and new_text.
  code_action_edit_change_item(
    const range_item& rnge    = range_item(),
    std::string       nw_text = std::string());

  /// Constructor from a JSON object,
  /// as received from the language server.
  code_action_edit_change_item(const boost::json::object& obj);

  /// Logs info about this class.
  std::stringstream log() const;

  /// Replaces the range in the given wxStyledTextCtrl with the new text
  /// specified in this item.
  /// Returns the difference in stc size caused by replacing.
  int replace_target(wxStyledTextCtrl* stc) const;

  std::string new_text; ///< replacement text
  range_item  range;    ///< range that will be replaced
};

/// Represents a code action edit, containing a map
/// of file with edit changes.
struct code_action_edit_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  code_action_edit_item(const boost::json::object& obj);

  std::map<std::string, std::vector<code_action_edit_change_item>>
    changes; ///< container with the changes
};

/// Represents a code action item (quick fix).
/// It contains a number of code_action_edit_items.
struct code_action_item
{
  /// Constructor from a JSON RPC object,
  /// as received from the language server.
  code_action_item(const json_rpc_message& msg);

  /// Logs info about this class.
  std::stringstream log() const;

  std::map<std::string, std::vector<code_action_edit_change_item>>
    changes; ///< container with the changes
};

/// Represents an element of a completion item.
struct completion_item_element
{
  /// Default constructor, taking insert text.
  completion_item_element(std::string text = std::string());

  /// Constructor from a JSON object,
  /// as received from the language server.
  completion_item_element(const boost::json::object& obj);

  const int kind{0}; ///< completion item kind (e.g., function, variable, etc.)

  const std::string detail, ///< details
    documentation,          ///< documentation
    insert_text;            ///< insert text
};

/// Represents a completion item.
struct completion_item
{
  /// Constructor, taking a position item, and a JSON
  /// object as received from the language server.
  completion_item(const position_item& p, const boost::json::object& obj);

  /// the pos is not received from the server, might not be
  /// necessary
  const position_item pos;

  std::vector<completion_item_element>
    elements; ///< container with completion_item_element
};

/// Represents a definition or implementation item.
struct definition_or_implementation_item
{
  /// Default constructor, taking a URI and a range.
  definition_or_implementation_item(
    std::string       u = std::string(),
    const range_item& r = range_item());

  /// Constructor from a JSON object,
  /// as received from the language server.
  definition_or_implementation_item(const boost::json::object& obj);

  const std::string uri; ///< uniform resource identifier

  const range_item range; ///< the range
};

/// Represents a single diagnostic item(error, warning, etc.).
struct diagnostic_item
{
  /// Default constructor, taking a range, and a message and severity.
  diagnostic_item(
    const range_item& r    = range_item(),
    std::string       msg  = std::string(),
    severity_t        type = severity_t::INFO);

  /// Constructor from a JSON object,
  /// as received from the language server.
  diagnostic_item(const boost::json::object& obj);

  /// Logs info about this class.
  std::stringstream log() const;

  const range_item range; ///< the range

  /// Severity of the diagnostic
  const severity_t severity{severity_t::INFO}; ///< severity level

  const std::string code, ///< code from json
    message,              ///< message from json
    source;               ///< source from json
};

/// Represents hover information.
struct hover_item
{
  /// Default constructor, taking a position and contents.
  hover_item(
    const position_item& p = position_item(),
    std::string          c = std::string());

  /// Constructor from a JSON object,
  /// as received from the language server.
  hover_item(const boost::json::object& obj);

  position_item pos; ///< position

  std::string contents, ///< contents from json
    kind;               ///< kind from json
};

/// Represents an inactive regions item.
struct inactive_regions_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  inactive_regions_item(const boost::json::object& obj);

  std::vector<range_item> ranges; ///< container with range_items
  const std::string       uri;    ///< uniform resource identifier
};

/// Represents an on-type formatting item, which specifies text changes to
/// be applied when a specific character is typed.
struct on_type_formatting_item : public code_action_edit_change_item
{
  /// Default constructor, taking a range and new_text.
  on_type_formatting_item(
    const range_item& rnge    = range_item(),
    std::string       nw_text = std::string());

  /// Constructor from a JSON object,
  /// as received from the language server.
  on_type_formatting_item(const boost::json::object& obj);
};

/// Represents a parameter of a callable-signature. A parameter can
/// have a label and a doc-comment.
struct parameter_information_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  parameter_information_item(const boost::json::object& obj);

  const std::string
      documentation; ///< the human-readable doc-comment of this parameter
  int label_start{0}, label_end{0}; ///< a label of type string
};

/// Represents a show or a log message item.
struct show_message_item
{
  /// Message type values.
  enum message_t
  {
    ERRORS = 1, ///< errors message
    WARNING,    ///< warning message
    INFO,       ///< info message
    LOG,        ///< log message
    DEBUG       ///< warning message, @since 3.18.0
  };

  /// Default constructor.
  show_message_item(
    /// message to be shown
    std::string msg = std::string(),
    /// the message type
    message_t t = INFO,
    /// show as wxInfoBar or log only
    bool is_show_item = true);

  /// Constructor from a JSON object,
  /// as received from the language server.
  show_message_item(
    /// the json object
    const boost::json::object& obj,
    /// default the message is shown as wxInfoBar
    bool is_show_item = true);

  const message_t   type;          ///< type from json
  const std::string message;       ///< message from json
  const bool        is_show{true}; ///< show as wxInfoBar or log only
};

/// Represents a signature information item,
/// the signature of something callable. A signature
/// can have a label, like a function-name, a doc-comment, and
/// a set of parameters.
struct signature_information_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  signature_information_item(const boost::json::object& obj);

  /// The parameters of this signature.
  std::vector<parameter_information_item> parameters;

  const std::string documentation; ///< the label of this signature
  const std::string label; ///< the human-readable doc-comment of this signature
};

/// Represents a signature help item.
/// Signature help represents the signature of something
/// callable. There can be multiple signature but only one
/// active and only one active parameter.
struct signature_help_item
{
  /// Constructor from a JSON object,
  /// as received from the language server.
  signature_help_item(const boost::json::object& obj);

  /// One or more signatures.
  std::vector<signature_information_item> signatures;

  bool is_error{false};

  const int active_signature{0}; ///< the active signature
  const int active_parameter{0}; ///< the active parameter
};

/// Convert a json value that has a key with a string value to a string.
/// Returns empty string if value cannot be converted.
std::string json_to_string(
  /// the value
  const boost::json::value& val,
  /// the key
  const std::string& key);

/// Type alias for a collection of code actions returned by the language
/// server.
using code_actions_t = code_action_item;

/// Type alias for collections of completions returned by the language server.
using completions_t = completion_item;

/// Type alias for a definition or implementation item returned by the
/// language server.
using definition_or_implementation_t =
  std::vector<definition_or_implementation_item>;

/// Type alias for a collection of diagnostics returned by the language
/// server.
using diagnostics_t = std::vector<diagnostic_item>;

/// Type alias for a hover item returned by the language server.
using hover_t = struct hover_item;

/// Type alias for on type format.
using on_type_formatting_item_t = std::vector<on_type_formatting_item>;
} // namespace wex
