project(wex-test-ctags)

file(GLOB SRCS ${SRCS} "*.cpp")

wex_test_app("wex-ctags")
