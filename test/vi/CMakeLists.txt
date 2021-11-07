project(wex-test-vi)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-vi wex-ui wex-common wex-data wex-factory wex-core")
