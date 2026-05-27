# LSP Implementation Documentation

## Phase 1: JSON-RPC Protocol & Diagnostics Storage ✅

### Overview

This document describes the Language Server Protocol (LSP) implementation for the wex editor. Phase 1 focuses on establishing the JSON-RPC 2.0 protocol layer and diagnostic storage infrastructure that will support all subsequent LSP functionality.

### Architecture

```
┌─────────────────────────────────────────────────────┐
│           LSP Client (wex::lsp::client)             │
│  Manages server connection and LSP protocol         │
└──────────────────┬──────────────────────────────────┘
                   │
        ┌──────────┴──────────┐
        │                     │
┌───────▼────────┐   ┌────────▼──────────┐
│  JSON-RPC      │   │  Diagnostics     │
│  Protocol      │   │  Storage         │
│  Handler       │   │                  │
└────────────────┘   └──────────────────┘
        │
        │ (stdio/pipe)
        │
┌───────▼──────────────────────┐
│  Language Server Process     │
│  (clangd, pyright, etc.)     │
└──────────────────────────────┘
```

### Key Components

#### 1. JSON-RPC 2.0 Protocol Handler (`json-rpc.h/cpp`)

The `json_rpc` class implements the JSON-RPC 2.0 specification for communication with language servers.

**Message Types:**

- **Request**: Has `id`, `method`, and `params`. Expects a response.
- **Notification**: Has `method` and `params`, but NO `id`. No response expected.
- **Response**: Has `id` and `result` (or `error`). Response to a request.
- **Error Response**: Has `id` and `error` object with `code` and `message`.

**Key Methods:**

```cpp
// Encoding (client -> server)
std::string encode_request(const std::string& method, 
                          const boost::json::object& params = {});
std::string encode_notification(const std::string& method,
                               const boost::json::object& params = {});
std::string encode_response(int id, const boost::json::object& result);
std::string encode_error(int id, int code, const std::string& message);

// Decoding (server -> client)
json_rpc_message decode(const std::string& data);

// Response handling
void register_handler(int id, response_handler handler);
void handle_response(const json_rpc_message& msg);
int next_id();
```

**Message Format Example:**

```
Content-Length: 63<CRLF>
<CRLF>
{"jsonrpc":"2.0","id":1,"method":"initialize","params":{}}
```

The Content-Length header specifies the byte length of the JSON payload and is required by the LSP specification.

#### 2. Diagnostic Storage (`diagnostics.h/cpp`)

The `diagnostics` class manages compilation errors, warnings, and hints from the language server.

**Data Structures:**

```cpp
struct diagnostic
{
  std::string message;
  std::string code;
  std::string source;
  severity_t severity;
  position_t range;  // start_line, start_character, end_line, end_character
};
```

**Key Methods:**

```cpp
void add(const std::string& uri, const diagnostic& diag);
void clear(const std::string& uri);
void clear_all();
const std::vector<diagnostic>& get(const std::string& uri) const;
std::vector<diagnostic> get_line(const std::string& uri, int line) const;
std::vector<std::string> get_uris() const;
size_t count() const;
bool has(const std::string& uri) const;
```

**Storage Structure:**

```
diagnostics {
  "file:///project/main.cpp" -> [diagnostic1, diagnostic2, ...]
  "file:///project/utils.cpp" -> [diagnostic3, ...]
}
```

### LSP Client (`client.h`)

The `client` class integrates JSON-RPC protocol handling with LSP-specific functionality.

**Key Features:**

- Process management (start/stop language server)
- Server capability tracking
- Request/response lifecycle management
- Notification handling

**Capabilities Struct:**

```cpp
struct capabilities
{
  bool hover_support;
  bool completion_support;
  bool definition_support;
  bool references_support;
  bool rename_support;
  bool formatting_support;
  bool diagnostic_support;
};
```

### File Structure

```
include/wex/lsp/
├── lsp.h                    # Main namespace header
├── client.h                 # LSP client interface
├── json-rpc.h              # JSON-RPC protocol
├── diagnostics.h           # Diagnostic types and storage

src/lsp/
├── CMakeLists.txt          # Build configuration
├── client.cpp              # Client implementation
├── diagnostics.cpp         # Diagnostic storage implementation
├── json-rpc.cpp            # JSON-RPC protocol implementation

test/lsp/
├── CMakeLists.txt          # Test build configuration
├── json-rpc-test.cpp       # JSON-RPC protocol tests
├── diagnostics-test.cpp    # Diagnostic storage tests
```

### Unit Tests

**JSON-RPC Tests** (`test/lsp/json-rpc-test.cpp`):
- ✅ `encode_request` - Verify request structure and ID assignment
- ✅ `encode_notification` - Verify notification format (no ID)
- ✅ `encode_response` - Verify response encoding
- ✅ `encode_error` - Verify error response with error codes
- ✅ `next_id_increments` - Verify auto-incrementing ID generation
- ✅ `decode_request` - Verify parsing incoming requests
- ✅ `decode_response` - Verify parsing server responses
- ✅ `decode_error` - Verify parsing error responses
- ✅ `register_and_handle_response` - Verify callback mechanism

**Diagnostics Tests** (`test/lsp/diagnostics-test.cpp`):
- ✅ `add_and_get` - Store and retrieve diagnostics
- ✅ `get_line_diagnostics` - Query by line number
- ✅ `clear_document` - Remove document diagnostics
- ✅ `clear_all` - Clear all diagnostics
- ✅ `get_uris` - List affected documents
- ✅ `multiple_documents` - Handle multiple URIs
- ✅ `severity_levels` - Store different severity levels
- ✅ `count_total_diagnostics` - Count total diagnostics

### Usage Example

```cpp
#include <wex/lsp/client.h>
#include <wex/lsp/json-rpc.h>

// Create JSON-RPC handler
wex::lsp::json_rpc rpc;

// Encode a request
boost::json::object params;
params["processId"] = 12345;
std::string message = rpc.encode_request("initialize", params);

// Handle response
wex::lsp::json_rpc_message response = rpc.decode(server_response);
if (response.is_response && !response.is_error) {
  // Extract capabilities from response.result
}

// Create LSP client
wex::lsp::client client("/usr/bin/clangd", "cpp");
client.initialize();

// Store diagnostics
wex::lsp::diagnostics diags;
wex::lsp::diagnostic d;
d.message = "Undeclared variable";
d.severity = wex::lsp::severity_t::ERROR;
diags.add("file:///test.cpp", d);
```

### LSP Specification References

- [JSON-RPC 2.0 Spec](https://www.jsonrpc.org/specification)
- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)

### Next Steps: Phase 2

Phase 2 will implement the actual LSP protocol methods:
- `initialize` / `shutdown` / `exit`
- `textDocument/didOpen` / `didChange` / `didClose`
- `textDocument/hover`
- `textDocument/completion`
- `textDocument/definition` / `references`
- `textDocument/publishDiagnostics` (server notification)
- `workspace/symbol`

### Build and Test

To build the LSP module:

```bash
cd build
cmake ..
make wex-lsp
make wex_test
```

To run tests:

```bash
./wex_test "[wex::lsp::json_rpc]"
./wex_test "[wex::lsp::diagnostics]"
```

### Dependencies

- **Boost.JSON**: JSON serialization/deserialization
- **Boost.Filesystem**: File path handling
- **Boost.Log**: Logging
- **wex-factory**: Process management for language server
