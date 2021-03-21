project(wex-test-del)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app()
