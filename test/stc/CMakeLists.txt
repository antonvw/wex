project(wex-test-stc)

file(GLOB SRCS ${SRCS} "*.cpp")
    
wex_test_app("wex-stc")
