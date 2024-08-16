project(wex-test-vi)

file(GLOB SRCS ${SRCS} "*.cpp" "../ex/test.cpp")

wex_test_app("wex-vi")
