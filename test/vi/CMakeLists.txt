project(wex-test-vi)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp" "../ex/test.cpp")
    
wex_test_app("wex-vi wex-ex wex-ui wex-common wex-data wex-factory wex-core")
