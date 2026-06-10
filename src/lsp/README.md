# LSP Implementation Guide for wex

## Overview

This library contains Language Server Process classes for wex.
It uses the ui and predecessor wex libraries (for the moment).

This document outlines the implementation strategy for Language Server Protocol (LSP) support in the wex library. LSP enables integration with language servers, providing features like:

- Diagnostics (errors, warnings, hints)
- Code completion
- Hover information
- Jump to definition
- Find references
- Rename refactoring
- Code formatting

### Design Principles

1. **Integration with existing infrastructure:**
   - Use `factory::process` for process management
   - Leverage `Boost.JSON` for JSON-RPC protocol
   - Use existing UI components for diagnostics display

2. **Event-driven architecture:**
   - Async communication with language servers
   - wxWidgets event system for notifications

3. **Extensible design:**
   - Support multiple language servers
   - Pluggable configuration

## Implementation Phases

### Phase 1: JSON-RPC Protocol Foundation

**Files to implement:**
- `src/lsp/client.cpp` - JSON-RPC message handling
- Unit tests for basic protocol operations

**Tasks:**
- [ ] Implement JSON-RPC 2.0 message encoding/decoding
- [ ] Create request/response tracking
- [ ] Handle async I/O with language server process
- [ ] Error handling and timeout management

**Key classes:**
- `wex::lsp::client` - Main client managing communication
- Message types for request/response/notification

### Phase 2: LSP Protocol Methods

**Implement core LSP methods:**
- [ ] `initialize` / `shutdown`
- [ ] `textDocument/didOpen`
- [ ] `textDocument/didChange`
- [ ] `textDocument/didClose`
- [ ] `textDocument/hover`
- [ ] `textDocument/completion`
- [ ] `textDocument/definition`
- [ ] `textDocument/publishDiagnostics` (receive)

**Files:**
- Extend `client.cpp` with full implementations
- Add message type definitions

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

## Integration Points

### With Existing Components

| Component | Usage | File |
|-----------|-------|------|
| `factory::process` | Spawn/manage server | `client.cpp` |
| `stc` | Display diagnostics | Phase 3 |
| `factory::frame` | Add LSP UI | Phase 4 |
| `data::*` | Configuration | Phase 5 |
| Event system | Async notifications | `client.cpp` |

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
