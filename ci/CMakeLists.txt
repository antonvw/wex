file(GLOB_RECURSE SRCS "../src/*.cpp")
file(GLOB_RECURSE INCLUDES "../include/*.h")

add_custom_target(codespell
  COMMAND codespell ${INCLUDES} ${SRCS}
    -L nd,te,INOUT,inout,defint,iif,hel -S "*.po,*.png,*.ico,*.icns,*.bin"
  COMMENT "codespell" VERBATIM)

add_custom_target(cppcheck
  COMMAND cppcheck
    --std=c++20
    --quiet --enable=all
    --suppress=constStatement 
    --suppress=missingIncludeSystem 
    --suppress=cppcheckError 
    --suppress=cstyleCast 
    --suppress=internalAstError 
    --suppress=noCopyConstructor 
    --suppress=noExplicitConstructor 
    --suppress=noOperatorEq 
    --suppress=ignoredReturnValue 
    --suppress=unknownMacro 
    --suppress=unusedScopedObject 
    --suppress=unusedFunction 
    --suppress=shadowArgument 
    --suppress=shadowVariable 
    --suppress=virtualCallInConstructor
    ${SRCS} -i LexAda.cxx -i odbc.cpp
  COMMENT "cppcheck" VERBATIM)

add_custom_target(cpplint
  COMMAND cpplint 
    --quiet --exclude=../src/lexers --filter=-build,-readability/casting,-readability/fn_size,-readability/multiline_string,-runtime/explicit,-runtime/indentation_namespace,-runtime/int,-runtime/references,-whitespace 
    ${SRCS}
  COMMENT "cpplint" VERBATIM)

add_custom_target(robocop
  COMMAND robocop
    -e 0201 -e 0202 -e 0308 -e 0309 -e 0505 -e 0704 -e 1003 -e 1004
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/app
  COMMENT "robocop" VERBATIM)
  
add_custom_target(analyse 
  DEPENDS cppcheck cpplint codespell robocop
  COMMENT "running checks" VERBATIM)
