project(wex-test-del)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-del wex-vcs wex-stc wex-vi wex-ex wex-ui wex-common wex-data wex-factory wex-core")
