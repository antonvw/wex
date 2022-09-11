project(wex-test-vcs)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp")
    
wex_test_app("wex-vcs")
