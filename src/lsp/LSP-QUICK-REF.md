# LSP Phase 1 Quick Reference Guide

## JSON-RPC Protocol Usage

### Creating and Sending Requests

```cpp
#include <wex/lsp/json-rpc.h>

wex::lsp::json_rpc rpc;

// Build parameters
boost::json::object params;
params["processId"] = 12345;
params["rootPath"] = "/home/user/project";

// Encode request (auto-generates ID)
std::string message = rpc.encode_request("initialize", params);
// Output: "Content-Length: 78\r\n\r\n{...json...}"

// Send to server via stdio/pipe
server_process.write(message);
```

### Handling Responses

```cpp
// Register callback for response with ID 1
rpc.register_handler(1, [](const wex::lsp::json_rpc_message& msg) {
    if (msg.is_error) {
        // Handle error
        auto error_code = msg.error["code"];
        auto error_msg = msg.error["message"];
    } else {
        // Handle success
        auto capabilities = msg.result["capabilities"];
    }
});

// Receive data from server
std::string server_response = server_process.read();

// Decode and dispatch to handler
wex::lsp::json_rpc_message msg = rpc.decode(server_response);
if (msg.is_response) {
    rpc.handle_response(msg);  // Calls registered handler
}
```

### Sending Notifications (No Response Expected)

```cpp
// Build parameters
boost::json::object params;
params["uri"] = "file:///test.cpp";
params["languageId"] = "cpp";
params["version"] = 1;
params["text"] = "int main() { return 0; }";

// Encode notification (no ID)
std::string message = rpc.encode_notification("textDocument/didOpen", params);

// Send to server (no need to register handler)
server_process.write(message);
```

### Sending Error Responses

```cpp
// When server sends malformed request
wex::lsp::json_rpc_message bad_msg = rpc.decode(malformed_data);

// Send error response
std::string error_response = rpc.encode_error(
    bad_msg.id,           // Echo back the request ID
    -32600,               // "Invalid Request" error code
    "Malformed JSON"
);

server_process.write(error_response);
```

## Diagnostic Storage Usage

### Adding Diagnostics

```cpp
#include <wex/lsp/diagnostics.h>

wex::lsp::diagnostics diags;

// Create a diagnostic
wex::lsp::diagnostic d;
d.message = "Undeclared variable 'x'";
d.code = "undeclared_var";
d.source = "clang";
d.severity = wex::lsp::severity_t::ERROR;
d.range.start_line = 5;
d.range.start_character = 10;
d.range.end_line = 5;
d.range.end_character = 11;

// Store it
diags.add("file:///project/main.cpp", d);
```

### Retrieving Diagnostics

```cpp
// Get all diagnostics for a file
auto file_diags = diags.get("file:///project/main.cpp");
for (const auto& d : file_diags) {
    std::cout << "Line " << d.range.start_line 
              << ": " << d.message << std::endl;
}

// Get diagnostics on a specific line
auto line_diags = diags.get_line("file:///project/main.cpp", 5);

// Check if document has diagnostics
if (diags.has("file:///project/main.cpp")) {
    // ...
}
```

### Clearing Diagnostics

```cpp
// Clear diagnostics for one file
diags.clear("file:///project/main.cpp");

// Clear all diagnostics
diags.clear_all();

// Get all affected files
auto uris = diags.get_uris();

// Get total count
size_t total = diags.count();
```

## Message Structures

### JSON-RPC Request

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "processId": 12345,
    "rootPath": "/home/user/project"
  }
}
```

### JSON-RPC Response (Success)

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "capabilities": {
      "hoverProvider": true,
      "completionProvider": {
        "resolveProvider": true
      }
    }
  }
}
```

### JSON-RPC Response (Error)

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32600,
    "message": "Invalid Request"
  }
}
```

### JSON-RPC Notification

```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/didOpen",
  "params": {
    "textDocument": {
      "uri": "file:///test.cpp",
      "languageId": "cpp",
      "version": 1,
      "text": "int main() { }"
    }
  }
}
```

## Severity Levels

```cpp
enum class severity_t
{
  ERROR = 1,      // Error (red)
  WARNING = 2,    // Warning (yellow)
  INFO = 3,       // Information (blue)
  HINT = 4        // Hint (gray)
};
```

## Error Codes (JSON-RPC Standard)

```cpp
const int PARSE_ERROR = -32700;           // Invalid JSON
const int INVALID_REQUEST = -32600;       // Request is not valid JSON-RPC
const int METHOD_NOT_FOUND = -32601;      // Method not found
const int INVALID_PARAMS = -32602;        // Invalid method params
const int INTERNAL_ERROR = -32603;        // Internal JSON-RPC error
const int SERVER_ERROR_START = -32099;    // Server errors (-32099 to -32000)
const int SERVER_NOT_INITIALIZED = -32002; // Server not initialized
const int UNKNOWN_ERROR_CODE = -32001;    // Unknown error code
```

## Common LSP Methods

### Lifecycle
- `initialize` - Start the server
- `shutdown` - Gracefully shut down
- `exit` - Terminate the connection

### Text Document Events
- `textDocument/didOpen` - Document opened
- `textDocument/didChange` - Document modified
- `textDocument/didClose` - Document closed
- `textDocument/didSave` - Document saved

### Server Notifications
- `textDocument/publishDiagnostics` - Server sends diagnostics

## Testing

### Run All Phase 1 Tests
```bash
cd build
./wex_test "[wex::lsp::]"
```

### Run Specific Test Suite
```bash
./wex_test "[wex::lsp::json_rpc]"
./wex_test "[wex::lsp::diagnostics]"
```

### Test Individual Test
```bash
./wex_test "[wex::lsp::json_rpc][encode_request]"
```

## Boost.JSON Quick Reference

### Creating Objects
```cpp
boost::json::object obj;
obj["key"] = "value";
obj["number"] = 42;
obj["null"] = nullptr;
obj["bool"] = true;
```

### Creating Arrays
```cpp
boost::json::array arr;
arr.push_back("item1");
arr.push_back(42);
arr.push_back(obj);
```

### Parsing JSON
```cpp
auto parsed = boost::json::parse(json_string);
auto obj = parsed.as_object();

std::string value = boost::json::value_to<std::string>(obj["key"]);
int number = boost::json::value_to<int>(obj["number"]);
```

### Serializing JSON
```cpp
std::string json_str = boost::json::serialize(obj);
```

## Troubleshooting

### Message Not Received
- Verify Content-Length header is correct
- Check JSON is valid
- Ensure CRLF line endings (\r\n)

### Decode Returns Empty Message
- Check message has Content-Length header
- Verify header format: "Content-Length: <number>\r\n\r\n"
- Ensure JSON payload follows header

### Handler Not Called
- Verify response ID matches registered handler ID
- Check response is marked as `is_response == true`
- Confirm handler is registered before response arrives

### Diagnostic Not Found
- Use exact URI string (case-sensitive)
- Check URI format: "file:///absolute/path"
- Verify line numbers are 0-based
