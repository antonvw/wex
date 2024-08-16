project(wex-test-stc)

file(GLOB SRCS ${SRCS} "*.cpp" "../vi/test.cpp")

wex_test_app("wex-stc")
