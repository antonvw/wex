project(wex-test-ex)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-ex wex-ui wex-common wex-data wex-factory wex-core")
