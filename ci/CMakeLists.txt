file(GLOB_RECURSE SRCS 
  ../src/*.cpp 
  ../src/syntax/wex/*.cpp 
  ../sample/*.cpp)
file(GLOB_RECURSE INCLUDES ../include/*.h)
file(GLOB MDS ../*.md)
file(GLOB_RECURSE MDSR ../doc/*.md ../src/*.md ../test/*.md)

if (NOT WIN32)
  add_custom_target(ctags-wex
    COMMAND ctags
      --c++-kinds=+p --languages=c,c++ --fields=+aiS -R
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/src
      ${CMAKE_SOURCE_DIR}/external/wxWidgets/include
      ${CMAKE_SOURCE_DIR}/external/ctags/libreadtags
      ${CMAKE_SOURCE_DIR}/external/pugixml/src  
    COMMAND
      /bin/mv tags $ENV{HOME}/.config/wex
      COMMENT "ctags sync" VERBATIM)

  add_custom_target(codespell
    COMMAND codespell ${INCLUDES} ${SRCS}
      -L nd,te,INOUT,inout,defint,iif,hel -S "*.po,*.png,*.ico,*.icns,*.bin"
    COMMENT "codespell" VERBATIM)

  add_custom_target(cppcheck
    COMMAND cppcheck 
      --std=c++${WEX_CXX_STANDARD}
      --quiet --enable=all
      --check-level=exhaustive
      --suppress=missingInclude
      --suppress=cppcheckError 
      --suppress=cstyleCast 
      --suppress=missingIncludeSystem
      --suppress=internalAstError 
      --suppress=invalidFunctionArg
      --suppress=noExplicitConstructor 
      --suppress=noOperatorEq 
      --suppress=ignoredReturnValue 
      --suppress=unknownMacro 
      --suppress=unmatchedSuppression 
      --suppress=unusedScopedObject 
      --suppress=unusedFunction 
      --suppress=shadowArgument 
      --suppress=shadowVariable 
      ${SRCS} -i odbc.cpp
    COMMENT "cppcheck" VERBATIM)

  add_custom_target(cpplint
    COMMAND cpplint 
      --quiet 
      --filter=-build,-readability/casting,-readability/fn_size,-readability/multiline_string,-runtime/explicit,-runtime/indentation_namespace,-runtime/int,-runtime/references,-whitespace
      ${SRCS}
    COMMENT "cpplint" VERBATIM)

  add_custom_target(markdownlint
    COMMAND markdownlint ${MDS} ${MDSR}
    COMMENT "markdownlint" VERBATIM)

  add_custom_target(robocop
    COMMAND robocop
      -e 0201 -e 0202 -e 0308 -e 0309 -e 0413 -e 0505 -e 0704
      -e 1003 -e 1004 -e 1005 -e 1006
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test/app
    COMMENT "robocop" VERBATIM)
    
  add_custom_target(analyse 
    DEPENDS codespell cppcheck cpplint markdownlint robocop
    COMMENT "running checks" VERBATIM)
endif()
