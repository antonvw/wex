////////////////////////////////////////////////////////////////////////////////
// Name:      client.cpp
// Purpose:   Implementation of class wex::lsp::capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/capabilities.h>

namespace wex
{
namespace lsp
{

boost::json_object capabilities::client() const
{
  const std::string text = R"(
  {
    "textDocument": 
    {
      "synchronization": 
      {
        "dynamicRegistration": true,
        "willSave": true,
        "willSaveWaitUntil": true,
        "didSave": true
      },
      "completion": 
      {
        "dynamicRegistration": true,
        "completionItem": {
          "snippetSupport": true,
          "commitCharactersSupport": true,
          "documentationFormat": "plaintext",
          "deprecatedSupport": true,
          "preselectSupport": true
        },
        "completionItemKind": 
        {
          "valueSet": [1, 2, 3, 4, 5, 6, 7]
        }
      },
      "hover":
      {
        "dynamicRegistration": true,
        "contentFormat": "plaintext"
      },
      "definition": 
      {
        "dynamicRegistration": true
      },
      "documentSymbol": {
        "dynamicRegistration": true,
        "symbolKind": {
          "valueSet": [1, 2, 3, 4]
        },
        "hierarchicalDocumentSymbolSupport": true
      },
    }
  }
  )";

  auto parsed = boost::json::parse(text);
  auto obj    = parsed.as_object();

  return obj;
}

bool capabilities::set(const boost::json_object& obj)
{
  if (obj.contains("hover"))
  {
    m_hover_support = true;
  }

  if (obj.contains("completion"))
  {
    m_completion_support = true;
  }
}
} // namespace lsp
} // namespace wex
