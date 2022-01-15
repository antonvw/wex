project(wex-lib VERSION ${WEX_VERSION})

if (CMAKE_BUILD_TYPE MATCHES "Coverage")
  add_custom_target(lcov-prep
    # prepare initial tracefile
    lcov --base-directory ./ --gcov-tool gcov --capture --initial --directory ./ --output-file app.base)
    
  add_custom_target(lcov
    # capture results
    COMMAND lcov --base-directory ./ --gcov-tool gcov --capture --directory ./ --output-file app.run
    # combine tracefiles
    COMMAND lcov --gcov-tool gcov --add-tracefile app.base --add-tracefile app.run --output-file app.run
    # remove output that we are not interested in
    COMMAND lcov --gcov-tool gcov --remove app.run "*/usr/*" "*sample*" "*external/*" "*/test/*" --output-file app.run
    # generate local files
    COMMAND genhtml --no-branch-coverage --title "${PROJECT_NAME}" app.run
    COMMENT "Running lcov" VERBATIM)    
endif ()

add_subdirectory(core)
add_subdirectory(factory)
add_subdirectory(data)
add_subdirectory(common)
add_subdirectory(ui)
add_subdirectory(vi)
add_subdirectory(stc)
add_subdirectory(del)

set_target_properties(
    wex-common wex-core wex-data wex-factory wex-ui wex-del wex-stc wex-vi
  PROPERTIES 
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

if (CMAKE_BUILD_TYPE EQUAL "Debug")
  set_target_properties(
    wex-common wex-core wex-data wex-factory wex-ui wex-del wex-stc wex-vi
    PROPERTIES DEBUG_POSTFIX "d")
endif ()
