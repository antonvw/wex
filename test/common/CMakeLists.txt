project(wex-test-common)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-common wex-data wex-factory wex-core")
