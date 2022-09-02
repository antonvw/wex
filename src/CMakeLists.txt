project(wex-lib VERSION ${WEX_VERSION})

if (CMAKE_BUILD_TYPE MATCHES "Coverage")
  set(wexBUILD_SAMPLES ON)
  set(wexBUILD_SAMPLES CACHE BOOL FORCE)
  set(wexBUILD_TESTS ON)
  set(wexBUILD_TESTS CACHE BOOL FORCE)
  
  add_custom_target(lcov-prep
    # prepare initial tracefile
    lcov 
      --base-directory ./ 
      --gcov-tool gcov 
      --capture 
      --initial 
      --directory ./ 
      --output-file app.base)
    
  add_custom_target(lcov
    # capture results
    COMMAND lcov 
      --base-directory ./ 
      --gcov-tool gcov 
      --capture 
      --directory ./ 
      --output-file app.run

    # combine tracefiles
    COMMAND lcov 
      --gcov-tool gcov 
      --add-tracefile app.base 
      --add-tracefile app.run 
      --output-file app.run

    # remove output that we are not interested in
    COMMAND lcov 
      --gcov-tool gcov 
      --remove app.run "*/usr/*" "*sample*" "*external/*" "*/test/*" "*factory/grid*" "include/wex/factory/text-window.h"
      --output-file app.run

    # generate local files
    COMMAND genhtml 
      # coverage rate limits for line coverage
      --rc genhtml_hi_limit=50
      --rc genhtml_med_limit=35
      # coverage rate limits for function coverage
      --rc genhtml_function_hi_limit=80
      --rc genhtml_function_med_limit=50
      --no-branch-coverage 
      --title "${PROJECT_NAME}" app.run

    COMMENT "Running lcov" VERBATIM)    
endif ()

add_subdirectory(core)
add_subdirectory(factory)
add_subdirectory(data)
add_subdirectory(common)
add_subdirectory(ui)
add_subdirectory(ex)
add_subdirectory(vi)
add_subdirectory(stc)
add_subdirectory(vcs)
add_subdirectory(del)

set_target_properties(
    wex-common wex-core wex-data wex-ex wex-factory wex-ui wex-del wex-stc wex-vi
  PROPERTIES 
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

if (CMAKE_BUILD_TYPE EQUAL "Debug")
  set_target_properties(
    wex-common wex-core wex-data wex-ex wex-factory wex-ui wex-del wex-stc wex-vi
    PROPERTIES DEBUG_POSTFIX "d")
endif ()
