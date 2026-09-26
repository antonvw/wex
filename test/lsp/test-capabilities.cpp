////////////////////////////////////////////////////////////////////////////////
// Name:      test-capabilities.cpp
// Purpose:   Unit tests for LSP capabilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lsp/capabilities.h>
#include <wex/test/test.h>
#include <wex/ui/menu.h>

TEST_CASE("wex::lsp::capabilities")
{
  wex::lsp::capabilities cap;

  SECTION("constructor")
  {
    REQUIRE(!cap.support(wex::lsp::capabilities::CAP_COMPLETION));
    REQUIRE(cap.trigger_completion_characters().empty());
    REQUIRE(cap.trigger_signature_characters().empty());
    REQUIRE(cap.trigger_character().empty());
    REQUIRE(cap.log().str().contains("completion"));
    auto menu = new wex::menu();
    REQUIRE(
      !cap.append_menu(menu, wex::lsp::capabilities::CAP_DECLARATION, 10));
  }

  SECTION("set")
  {
    // clang-format off
    // this is a log of clangd capabilities, slightly modified triffer characters
    const std::string text = R"(
    {"astProvider":true,
     "callHierarchyProvider":true,
     "clangdInlayHintsProvider":true,
     "codeActionProvider":true,
     "compilationDatabase":{"automaticReload":true},
     "completionProvider":
       {"resolveProvider":false,"triggerCharacters":[".","<",">",":","/","*"]},
     "declarationProvider":true,
     "definitionProvider":true,
     "documentFormattingProvider":true,
     "documentHighlightProvider":true,
     "documentLinkProvider":{"resolveProvider":false},
     "documentOnTypeFormattingProvider":
       {"firstTriggerCharacter":"n","moreTriggerCharacter":[]},
     "documentRangeFormattingProvider":{"rangesSupport":true},
     "documentSymbolProvider":true,
     "executeCommandProvider":
        {"commands":["clangd.applyFix","clangd.applyRename","clangd.applyTweak"]},
     "foldingRangeProvider":true,
     "hoverProvider":true,
     "implementationProvider":true,
     "inactiveRegionsProvider":true,
     "inlayHintProvider":true,
     "memoryUsageProvider":true,
     "referencesProvider":true,
     "renameProvider":true,
     "selectionRangeProvider":true,
     "semanticTokensProvider":
       {"full":{"delta":true},"legend":{"tokenModifiers":["declaration","definition","deprecated","deduced","readonly","static","abstract","virtual","dependentName","defaultLibrary","usedAsMutableReference","usedAsMutablePointer","constructorOrDestructor","userDefined","functionScope","classScope","fileScope","globalScope"],"tokenTypes":["variable","variable","parameter","function","method","function","property","variable","class","interface","enum","enumMember","type","type","unknown","namespace","typeParameter","concept","type","macro","modifier","operator","bracket","label","comment"]},"range":false},
     "signatureHelpProvider":
        {"triggerCharacters":[",","<",">",","]},
     "standardTypeHierarchyProvider":true,"textDocumentSync":{"change":2,"openClose":true,"save":true},
     "typeDefinitionProvider":true,
     "typeHierarchyProvider":true,
     "workspaceSymbolProvider":true}
    )";
    // clang-format on
    auto parsed = boost::json::parse(text);

    REQUIRE(cap.set(parsed.as_object()));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_COMPLETION));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_DECLARATION));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_DEFINITION));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_FORMATTING));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_HOVER));
    REQUIRE(cap.support(wex::lsp::capabilities::CAP_IMPLEMENTATION));

    auto menu = new wex::menu();
    REQUIRE(cap.append_menu(menu, wex::lsp::capabilities::CAP_DECLARATION, 10));
  }
}
