project(wex-test-data)

file(GLOB SRCS ${SRCS} "*.cpp")
    
wex_test_app("wex-data")
