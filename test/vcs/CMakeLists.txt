project(wex-test-vcs)

file(GLOB SRCS ${SRCS} "*.cpp" "../stc/test.cpp")

wex_test_app("wex-vcs")
