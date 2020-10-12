project(wex-test-report)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app()
