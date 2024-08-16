project(wex-test-del)

file(GLOB SRCS ${SRCS} "*.cpp" "../vcs/test.cpp")

wex_test_app("wex-del")
