# Phase 2 Implementation Plan: LSP Protocol Methods

## Overview

Phase 2 builds on the Phase 1 foundation (JSON-RPC protocol and diagnostics storage) to implement the core Language Server Protocol methods required for editor-server communication.

## Architecture

```txt
Phase 1 Foundation (✅ Complete)
├── JSON-RPC Protocol Handler
├── Diagnostic Storage
└── Client Scaffold

         ↓

Phase 2: Protocol Methods (📋 This Phase)
├── Lifecycle Management
│   ├── initialize
│   ├── shutdown
│   └── exit
├── Document Management
│   ├── textDocument/didOpen
│   ├── textDocument/didChange
│   ├── textDocument/didClose
│   └── textDocument/didSave
├── Server Notifications
│   └── textDocument/publishDiagnostics
└── Feature Requests (Basic)
    ├── textDocument/hover
    ├── textDocument/completion
    └── textDocument/definition
```

## Detailed Tasks

### 1. Lifecycle Management

#### 1.1 Initialize Method

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
bool client::initialize()
```

**Implementation:**
- Build `initialize` request with:
  - `processId` - PID of wex editor
  - `rootPath` - Project root directory
  - `capabilities` - Client capabilities
- Send request to server
- Register response handler
- Parse server capabilities response
- Store capabilities in `m_capabilities` struct
- Set `m_initialized = true` on success

**Request Example:**
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "processId": 12345,
    "rootPath": "/home/user/project",
    "capabilities": {
      "textDocument": {
        "synchronization": { "didSave": true }
      }
    }
  }
}
```

**Response Handling:**
```cpp
m_rpc.register_handler(1, [this](const json_rpc_message& msg) {
    if (!msg.is_error && msg.result.count("capabilities")) {
        auto caps = msg.result["capabilities"];
        m_capabilities.hover_support = /* ... */;
        m_capabilities.completion_support = /* ... */;
        // ... store other capabilities
        m_initialized = true;
    }
});
```

**Tests to Add:**
- Test initialization success
- Test capabilities parsing
- Test initialization with missing capabilities
- Test re-initialization handling

#### 1.2 Shutdown Method

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
bool client::shutdown()
```

**Implementation:**
- Send `shutdown` request
- Wait for response
- Gracefully close connection
- Return success/failure

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "shutdown"
}
```

#### 1.3 Exit Notification

**File:** `src/lsp/client.cpp`

**Implementation:**
- Send `exit` notification
- Terminate server process
- Clean up resources

### 2. Document Management

#### 2.1 Text Document Did Open

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
bool client::did_open(
  const std::string& uri,
  const std::string& language_id,
  const std::string& text)
```

**Implementation:**
- Build `textDocument/didOpen` notification
- Include document URI, language ID, version, content
- Send to server (no response handler needed)
- Track open document internally

**Notification:**
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/didOpen",
  "params": {
    "textDocument": {
      "uri": "file:///project/main.cpp",
      "languageId": "cpp",
      "version": 1,
      "text": "int main() { return 0; }"
    }
  }
}
```

**Tests:**
- Test open notification format
- Test multiple documents
- Test URI encoding

#### 2.2 Text Document Did Change

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
bool client::did_change(const std::string& uri, const std::string& text)
```

**Implementation:**
- Build `textDocument/didChange` notification
- Include URI, updated content, version
- Send to server
- Increment document version number

**Notification:**
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/didChange",
  "params": {
    "textDocument": {
      "uri": "file:///project/main.cpp",
      "version": 2
    },
    "contentChanges": [
      {
        "text": "int main() { int x = 42; return 0; }"
      }
    ]
  }
}
```

#### 2.3 Text Document Did Close

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
bool client::did_close(const std::string& uri)
```

**Implementation:**
- Build `textDocument/didClose` notification
- Send to server
- Clear diagnostics for document
- Remove from tracked documents

### 3. Server Notifications

#### 3.1 Publish Diagnostics Handler

**File:** `src/lsp/client.cpp` or new `src/lsp/notification-handler.cpp`

**Method:** `textDocument/publishDiagnostics` (incoming)

**Implementation:**
- Listen for diagnostics notifications from server
- Parse diagnostic array from notification params
- Store in `diagnostics` storage
- Trigger UI update callback

**Notification from Server:**
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/publishDiagnostics",
  "params": {
    "uri": "file:///project/main.cpp",
    "diagnostics": [
      {
        "range": {
          "start": { "line": 5, "character": 10 },
          "end": { "line": 5, "character": 15 }
        },
        "severity": 1,
        "code": "undeclared",
        "source": "clang",
        "message": "Use of undeclared identifier 'x'"
      }
    ]
  }
}
```

**Handler Logic:**
```cpp
void handle_publish_diagnostics(const json_rpc_message& notification) {
    auto uri = notification.params["uri"].as_string();
    auto diags_array = notification.params["diagnostics"].as_array();
    
    m_diagnostics.clear(uri);  // Clear old diagnostics
    
    for (const auto& diag_json : diags_array) {
        wex::lsp::diagnostic diag;
        // Parse and store diagnostic
        m_diagnostics.add(uri, diag);
    }
    
    // Notify UI about diagnostics update
    on_diagnostics_updated(uri);
}
```

**Tests:**
- Test diagnostic parsing
- Test multiple diagnostics per file
- Test diagnostic clearing
- Test diagnostic update callback

### 4. Feature Request Methods (Basic)

#### 4.1 Hover Request

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
std::string client::hover(
  const std::string& uri,
  int line,
  int character)
```

**Implementation:**
- Build `textDocument/hover` request
- Include position (line, character)
- Send request
- Parse response for hover content
- Return markdown/plaintext content

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "textDocument/hover",
  "params": {
    "textDocument": { "uri": "file:///project/main.cpp" },
    "position": { "line": 5, "character": 10 }
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "contents": "int x\n\nVariable declaration"
  }
}
```

#### 4.2 Completion Request

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
std::vector<std::string> client::completion(
  const std::string& uri,
  int line,
  int character)
```

**Implementation:**
- Build `textDocument/completion` request
- Send with position
- Parse completion items from response
- Return list of completion strings

**Request:**
```json
{
  "jsonrpc": "2.0",
  "id": 4,
  "method": "textDocument/completion",
  "params": {
    "textDocument": { "uri": "file:///project/main.cpp" },
    "position": { "line": 5, "character": 10 }
  }
}
```

**Response:**
```json
{
  "jsonrpc": "2.0",
  "id": 4,
  "result": {
    "isIncomplete": false,
    "items": [
      {
        "label": "function_name",
        "kind": 6,
        "detail": "() -> void"
      }
    ]
  }
}
```

#### 4.3 Definition Request

**File:** `src/lsp/client.cpp`

**Signature:**
```cpp
std::string client::definition(
  const std::string& uri,
  int line,
  int character)
```

**Implementation:**
- Build `textDocument/definition` request
- Parse location from response (URI + position)
- Return as formatted string or Location struct
- Allow jumping to definition in editor

## Implementation Order

### Priority 1 (Must Have)
1. `initialize()` - Required to connect with server
2. `shutdown()` / `exit()` - Required for clean shutdown
3. `did_open()` - Required to track documents
4. `did_change()` - Required for live updates
5. `did_close()` - Required for cleanup
6. `publishDiagnostics` handler - Required for error reporting

### Priority 2 (High Value)
7. `hover()` - Tooltip information
8. `completion()` - Code completion
9. `definition()` - Go to definition

### Priority 3 (Future)
- `references()` - Find usages
- `rename()` - Refactoring
- `formatting()` - Code formatting
- `workspace/symbol` - Global symbol search

## Testing Strategy

### Unit Tests

Create `test/lsp/client-test.cpp`:

```cpp
TEST_CASE("wex::lsp::client")
{
  // Setup mock server/process
  wex::lsp::client client("/path/to/server", "cpp");
  
  SECTION("initialize sends correct request") {
    // Verify request format
    // Verify capabilities parsing
  }
  
  SECTION("did_open sends notification") {
    // Verify notification format
  }
  
  SECTION("did_change updates document") {
    // Verify change notification
  }
  
  SECTION("did_close cleans up") {
    // Verify close notification
    // Verify diagnostics cleared
  }
  
  SECTION("publishDiagnostics stores diagnostics") {
    // Verify diagnostic storage
    // Verify callback invocation
  }
  
  SECTION("hover returns content") {
    // Verify request format
    // Verify response parsing
  }
}
```

### Integration Tests

- Test full initialize→didOpen→didChange→publishDiagnostics→shutdown flow
- Test with real language server (e.g., clangd)
- Test error handling and recovery

## Modified Files

- `src/lsp/client.cpp` - Implement all methods
- `include/wex/lsp/client.h` - Add private helper methods/members
- `test/lsp/client-test.cpp` - New test file
- `test/lsp/CMakeLists.txt` - Add client tests

## New Headers/Utilities (Optional)

Consider creating:
- `include/wex/lsp/protocol-methods.h` - Method implementations
- `include/wex/lsp/notification-handler.h` - Server notification handling
- `include/wex/lsp/types.h` - LSP data types (Position, Range, Location, etc.)

## Build & Test

```bash
# Build
cmake ..
make wex-lsp

# Test
./wex_test "[wex::lsp::client]"
./wex_test "[wex::lsp::]"  # All LSP tests
```

## Deliverables

- ✅ Lifecycle methods (initialize, shutdown, exit)
- ✅ Document management (didOpen, didChange, didClose)
- ✅ Diagnostic publishing handler
- ✅ Basic feature requests (hover, completion, definition)
- ✅ Comprehensive unit tests
- ✅ Updated documentation
- ✅ Build integration

## Acceptance Criteria

- All methods implemented and tested
- Can establish connection with real LSP server
- Can open/edit/close documents
- Can receive and display diagnostics
- Can request hover information
- Can request completions
- All tests passing
- No compiler warnings
- Full documentation

## Estimated Effort

- Implementation: 4-6 hours
- Testing: 2-3 hours
- Documentation: 1-2 hours
- **Total: ~12 hours**

## Dependencies

- Phase 1 foundation (✅ Complete)
- Boost libraries (existing)
- Mock language server for testing (clangd, pyright, etc.)

---

**Status:** Ready to begin Phase 2 implementation
**Branch:** `lsp-support`
**Next:** Implementation of lifecycle and document management methods
