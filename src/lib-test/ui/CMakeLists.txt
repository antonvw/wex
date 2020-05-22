project(wex-test-ui)

file(GLOB SRCS ${SRCS} "*.cpp" "../*.cpp" 
  "../common/*.cpp" 
  "../data-src/*.cpp"
  "../lexer/*.cpp"
  "../stc/*.cpp"
  "../ui/*.cpp"
  "../vcs/*.cpp"
  "../vi/*.cpp")
    
test_app()
