project(wex-test-ui)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app()
