project(wex-test-common)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-data wex-common wex-factory wex-core")
