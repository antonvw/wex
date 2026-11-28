////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/lsp/capabilities.h>
#include <wex/ui/menu.h>

namespace wex
{

void trigger_characters_log(
  const std::string&              name,
  const std::vector<std::string>& chars,
  std::stringstream&              ss)
{
  if (!chars.empty())
  {
    ss << "trigger " << name << ": ";

    for (const auto& tc : chars)
    {
      ss << "'" << tc << "' ";
    }
  }
}

void trigger_characters_set(
  const boost::json::object& obj,
  std::vector<std::string>&  chars)
{
  if (auto tc = obj.if_contains("triggerCharacters"); tc && tc->is_array())
  {
    for (const auto& v : tc->as_array())
    {
      if (v.is_string())
      {
        chars.emplace_back(v.as_string().c_str());
      }
    }
  }
}

namespace lsp
{
std::vector<std::string> capabilities::m_support_info =
  // must match the capabilities_t
  {"completion",
   "declaration",
   "definition",
   "formatting",
   "hover",
   "implementation",
   "signature-help"};

bool capabilities::append_menu(
  wex::menu*                              menu,
  const std::vector<std::pair<int, int>>& v) const
{
  bool added = false;

  for (const auto& el : v)
  {
    if (support(el.first))
    {
      if (!added)
      {
        menu->append({{}});
      }

      if (menu->append({{el.second, "Goto " + m_support_info[el.first]}}))
      {
        added = true;
      }
    }
  }

  return added;
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
        "codeActionLiteralSupport":
        {
          "codeActionKind":
          {
            "valueSet":
            [
              "",
              "quickfix",
              "refactor",
              "refactor.extract",
              "refactor.inline",
              "refactor.rewrite",
              "source",
              "source.organizeImports",
              "source.fixAll"
            ]
          }
        },
        "declaration":
        {
          "dynamicRegistration": false
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
        "implementation":
        {
          "dynamicRegistration": false
        },
        "inactiveRegionsCapabilities":
        {
          "inactiveRegions": true
        },
        "signatureHelp":
        {
          "dynamicRegistration": false,
          "signatureInformation":
          {
            "documentationFormat": ["markdown", "plaintext"],
            "parameterInformation":
            {
              "labelOffsetSupport": true
            },
            "activeParameterSupport": true
          },
          "contextSupport": true
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

  trigger_characters_log("completion", m_trigger_completion_characters, ss);

  trigger_characters_log(
    "signature-help",
    m_trigger_signature_help_characters,
    ss);

  if (!m_first_trigger_character.empty())
  {
    ss << "first_trigger_character: " << m_first_trigger_character;
  }

  return ss;
}

bool capabilities::set(const boost::json::object& obj)
{
  m_support.reset();
  m_first_trigger_character.clear();
  m_trigger_completion_characters.clear();
  m_trigger_signature_help_characters.clear();

  if (auto it = obj.if_contains("completionProvider"); it && it->is_object())
  {
    m_support.set(CAP_COMPLETION);

    trigger_characters_set(it->as_object(), m_trigger_completion_characters);
  }

  if (obj.contains("declarationProvider"))
  {
    m_support.set(CAP_DECLARATION);
  }

  if (obj.contains("definitionProvider"))
  {
    m_support.set(CAP_DEFINITION);
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

  if (obj.contains("hoverProvider"))
  {
    m_support.set(CAP_HOVER);
  }

  if (obj.contains("implementationProvider"))
  {
    m_support.set(CAP_IMPLEMENTATION);
  }

  if (auto it = obj.if_contains("signatureHelpProvider"); it && it->is_object())
  {
    m_support.set(CAP_SIGNATURE_HELP);

    trigger_characters_set(
      it->as_object(),
      m_trigger_signature_help_characters);
  }

  return !m_support.none();
}

bool capabilities::support(size_t cap) const
{
  return cap < m_support.size() && m_support.test(cap);
}
} // namespace lsp
} // namespace wex
