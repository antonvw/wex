////////////////////////////////////////////////////////////////////////////////
// Name:      capabilities.h
// Purpose:   Declaration of class wex::lsp::capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <boost/json.hpp>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace wex
{
class menu;

namespace lsp
{
/// Server capabilities tracking.
class capabilities
{
public:
  /// Flags for capability support.
  enum
  {
    CAP_COMPLETION = 0, ///< completion capability
    CAP_DECLARATION,    ///< declaration capability
    CAP_DEFINITION,     ///< denifition capability
    CAP_FORMATTING,     ///< formatting capability
    CAP_HOVER,          ///< hover capability
    CAP_IMPLEMENTATION, ///< implementation capability
  };

  /// Default constructor.
  /// Initialized the support info.
  capabilities();

  /// Appends a menu entries, depending on capability for it.
  /// The menu text is derived from the support info, with Goto prefixed.
  /// Returns true if entry was appended.
  bool append_menu(
    /// menu to append to
    wex::menu* menu,
    /// vector of capabilities and event id's
    const std::vector<std::pair<
      /// required capability
      int,
      /// event id that will be done if selected
      int>>& v) const;

  /// A typedef containing capability flags.
  using capabilities_t = std::bitset<6>;

  /// Returns the wex client capabilities.
  boost::json::object client() const;

  /// Logs info about this class.
  std::stringstream log() const;

  /// Parses the server capabilites, and fills members.
  bool set(const boost::json::object& obj);

  /// Returns true if server supports specified capability.
  bool support(size_t cap) const;

  /// Returns the trigger completion characters.
  const std::vector<std::string>& trigger_completion_characters() const
  {
    return m_trigger_completion_characters;
  }

  /// Returns the trigger character for on type formatting.
  const std::string trigger_character() const
  {
    return m_first_trigger_character;
  }

  /// Returns the trigger signature characters.
  const std::vector<std::string>& trigger_signature_characters() const
  {
    return m_trigger_signature_characters;
  }

private:
  capabilities_t                         m_support{0};
  static inline std::vector<std::string> m_support_info;

  std::vector<std::string> m_trigger_completion_characters;
  std::vector<std::string> m_trigger_signature_characters;

  std::string m_first_trigger_character;
};

} // namespace lsp
} // namespace wex
