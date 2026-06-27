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
#include <vector>

namespace wex
{
namespace lsp
{
/// Server capabilities tracking.
class capabilities
{
public:
  /// Flags for capability support.
  enum
  {
    CAP_COMPLETION = 0,
    CAP_DEFINITION,
    CAP_HOVER,
  };

  /// Default constructor.
  capabilities();

  /// A typedef containing capability flags.
  using capabilities_t = std::bitset<3>;

  /// Returns the client capabilities.
  boost::json::object client() const;

  /// Logs info about this class.
  std::stringstream log() const;

  /// Parses the server capabilites, and fills members.
  bool set(const boost::json::object& obj);

  /// Returns true if server supports specified type.
  bool support(size_t cap) const;

  /// Returns the trigger completion characters.
  const std::vector<std::string>& trigger_completion_characters() const
  {
    return m_trigger_completion_characters;
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
};

} // namespace lsp
} // namespace wex
