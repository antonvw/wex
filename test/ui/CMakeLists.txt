project(wex-test-ui)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-ui wex-common wex-data wex-factory wex-core")
