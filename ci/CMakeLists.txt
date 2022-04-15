file(GLOB_RECURSE SRCS "../src/*.cpp")
file(GLOB_RECURSE INCLUDES "../include/*.h")

add_custom_target(analyse
  COMMAND codespell ${INCLUDES} ${SRCS}
    -L nd,te,INOUT,inout,defint,iif,hel -S "*.po,*.png,*.ico,*.icns,*.bin"

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

  COMMAND cpplint 
    --quiet --exclude=../src/lexers --filter=-build,-readability/casting,-readability/fn_size,-readability/multiline_string,-runtime/explicit,-runtime/indentation_namespace,-runtime/int,-runtime/references,-whitespace 
    ${SRCS}

  COMMENT "Running checks" VERBATIM)
