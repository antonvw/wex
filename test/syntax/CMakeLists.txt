project(wex-test-syntax)

file(GLOB SRCS ${SRCS} "*.cpp")
    
wex_test_app("wex-syntax")
