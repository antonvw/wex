# LSP Implementation Guide for wex

## Overview

This library contains Language Server Process classes for wex.
It uses the ui and predecessor wex libraries (for the moment).

This document outlines the implementation strategy for Language Server Protocol (LSP) 
support in the wex library. LSP enables integration with language servers, 
providing features like:

- Diagnostics (errors, warnings, hints)
- Code completion
- Hover information
- Jump to definition
- Find references
- Rename refactoring
- Code formatting

### Design Principles

1. **Integration with existing infrastructure:**
   - Use `boost::process` for process management
   - Leverage `boost.JSON` for JSON-RPC protocol
   - Use existing UI components for diagnostics display

2. **Event-driven architecture:**
   - Async communication with language servers
   - wxWidgets event system for notifications

3. **Extensible design:**
   - Support multiple language servers
   - Pluggable configuration

## Implementation Phases

**Key classes:**
- `wex::lsp::client` - Main client managing communication
- Message types for request/response/notification

### Phase 3: Diagnostics Integration

**Tasks:**
- [ ] Receive diagnostics from server via `publishDiagnostics`
- [ ] Display diagnostics as markers/indicators in `stc`
- [ ] Link diagnostics to editor line numbers
- [ ] Create diagnostics panel/list

**Integration points:**
- `wex::syntax::stc` - Add diagnostic markers
- `wex::syntax::indicator` - Visual feedback

### Phase 4: UI Components

**Features:**
- [ ] Completion popup (using factory components)
- [ ] Hover tooltip display
- [ ] Diagnostics inline display
- [ ] Server status indicator in statusbar

**Integration points:**
- `wex::factory::frame` - Add LSP UI elements
- `wex::ui::` - Dialog components

### Phase 5: Configuration & Server Management

**Features:**
- [ ] Server configuration dialog
- [ ] Per-language server settings
- [ ] Auto-start/restart logic
- [ ] Multiple concurrent servers

**Integration points:**
- `wex::data::` - Configuration data structures
- `wex::config::` - Configuration storage

## JSON-RPC Example

```cpp
// Sending a request
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "processId": 1234,
    "rootPath": "/home/user/project",
    "capabilities": {}
  }
}

// Receiving a response
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "capabilities": {
      "hoverProvider": true,
      "completionProvider": {"triggerCharacters": ["."]},
      "definitionProvider": true
    }
  }
}

// Receiving a notification
{
  "jsonrpc": "2.0",
  "method": "textDocument/publishDiagnostics",
  "params": {
    "uri": "file:///home/user/project/main.cpp",
    "diagnostics": [{
      "range": {"start": {"line": 5, "character": 10}, "end": {...}},
      "severity": 1,
      "message": "Variable not declared"
    }]
  }
}
```

## References

- [LSP Specification](https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/)
- [LSP Examples](https://github.com/microsoft/language-server-protocol/tree/gh-pages/_specifications)
- [JSON-RPC 2.0 Spec](https://www.jsonrpc.org/specification)
- Wex existing patterns: `vcs/process.h`, `factory/frame.h`
