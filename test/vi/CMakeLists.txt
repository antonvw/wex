project(wex-test-vi)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app()
