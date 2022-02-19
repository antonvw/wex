project(wex-test-del)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-del wex-stc wex-vi wex-ui wex-common wex-data wex-factory wex-core")
