////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/lsp/capabilities.h>

#include <iostream>

namespace wex
{
namespace lsp
{
capabilities::capabilities()
{
  if (m_support_info.empty())
  {
    // must match the capabilities_t
    m_support_info.emplace_back("completion");
    m_support_info.emplace_back("definition");
    m_support_info.emplace_back("formatting");
    m_support_info.emplace_back("hover");
  }
}

boost::json::object capabilities::client() const
{
  try
  {
    const std::string text = R"(
    {
      "textDocument":
      {
        "completion":
        {
          "dynamicRegistration": false,
          "completionItem":
          {
            "snippetSupport": false,
            "commitCharactersSupport": true,
            "documentationFormat": ["plaintext"],
            "deprecatedSupport": true,
            "preselectSupport": true
          },
          "completionItemKind":
          {
            "valueSet": [1, 2, 3, 4, 5, 6, 7]
          }
        },
        "definition":
        {
          "dynamicRegistration": false
        },
        "documentSymbol":
        {
          "dynamicRegistration": false,
          "symbolKind":
          {
            "valueSet": [1, 2, 3, 4]
          },
          "hierarchicalDocumentSymbolSupport": true
        },
        "hover":
        {
          "dynamicRegistration": false,
          "contentFormat": ["plaintext"]
        },
        "synchronization":
        {
          "dynamicRegistration": false,
          "didSave": true,
          "change":
          {
            "dynamicRegistration": false,
            "willSynchronize": true,
            "incremental": true
          }
        }
      }
    }
    )";

    auto parsed = boost::json::parse(text);
    auto obj    = parsed.as_object();
    return obj;
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "wex::lsp::capabilities::client()";
  }

  return boost::json::object();
}

std::stringstream capabilities::log() const
{
  std::stringstream ss;
  size_t            cap = 0;

  for (const auto& inf : m_support_info)
  {
    ss << inf << ": " << m_support.test(cap++) << " ";
  }

  if (!m_trigger_completion_characters.empty())
  {
    ss << "trigger completion characters: ";

    for (const auto& tc : m_trigger_completion_characters)
    {
      ss << "'" << tc << "' ";
    }
  }

  if (!m_trigger_signature_characters.empty())
  {
    ss << "trigger signature characters: ";

    for (const auto& tc : m_trigger_signature_characters)
    {
      ss << "'" << tc << "' ";
    }
  }

  if (!m_first_trigger_character.empty())
  {
    ss << "first_trigger_character: " << m_first_trigger_character;
  }

  return ss;
}

bool capabilities::set(const boost::json::object& obj)
{
  m_support.reset();
  m_trigger_completion_characters.clear();
  m_trigger_signature_characters.clear();

  if (auto it = obj.if_contains("completionProvider"); it && it->is_object())
  {
    m_support.set(CAP_COMPLETION);

    const auto& cp = it->as_object();

    if (auto tc = cp.if_contains("triggerCharacters"); tc && tc->is_array())
    {
      for (const auto& v : tc->as_array())
      {
        if (v.is_string())
        {
          m_trigger_completion_characters.emplace_back(v.as_string().c_str());
        }
      }
    }
  }

  if (obj.contains("definitionProvider"))
  {
    m_support.set(CAP_DEFINITION);
  }

  if (obj.contains("hoverProvider"))
  {
    m_support.set(CAP_HOVER);
  }

  if (obj.contains("documentOnTypeFormattingProvider"))
  {
    m_support.set(CAP_FORMATTING);

    if (
      auto it = obj.at("documentOnTypeFormattingProvider");
      it.as_object().contains("firstTriggerCharacter"))
    {
      m_first_trigger_character =
        it.as_object().at("firstTriggerCharacter").as_string();
    }
  }

  return true;
}

bool capabilities::support(size_t cap) const
{
  return cap < m_support.size() && m_support.test(cap);
}
} // namespace lsp
} // namespace wex
