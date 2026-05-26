project(wex-lsp)

set(SRCS client.cpp diagnostics.cpp json-rpc.cpp)

add_library(wex-lsp ${SRCS})

target_include_directories(wex-lsp PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(
  wex-lsp
  PRIVATE
    wex-factory
    Boost::json
    Boost::filesystem
    Boost::log
)

set_target_properties(
  wex-lsp
  PROPERTIES
    CXX_STANDARD ${WEX_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
)
