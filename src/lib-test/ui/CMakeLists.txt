project(wex-test-ui)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp" 
  "../common/*.cpp" 
  "../data-src/*.cpp"
  "../stc/*.cpp"
  "../ui/*.cpp"
  "../vi/*.cpp")
    
wex_test_app()
