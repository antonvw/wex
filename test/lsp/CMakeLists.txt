project(wex-test-lsp)

file(GLOB SRCS ${SRCS} "*.cpp")

wex_test_app("wex-lsp")
