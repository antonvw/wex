project(wex-test-data)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-data wex-factory wex-core")
