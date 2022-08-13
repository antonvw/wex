project(wex-test-stc)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-stc wex-vi wex-ex wex-ui wex-common wex-data wex-factory wex-core")
