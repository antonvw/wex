project(wex-lib VERSION ${WEX_VERSION})

if(CMAKE_BUILD_TYPE MATCHES "Coverage")
  set(wexBUILD_SAMPLES ON)
  set(wexBUILD_SAMPLES CACHE BOOL FORCE)
  set(wexBUILD_TESTS ON)
  set(wexBUILD_TESTS CACHE BOOL FORCE)

  if(wexGCOV)
    set(wexGCOV_USE ${wexGCOV})
  else()
    set(wexGCOV_USE "gcov")
  endif()

  add_custom_target(
    lcov-prep
    # prepare initial tracefile
    lcov --ignore-errors mismatch --base-directory ./ --gcov-tool ${wexGCOV_USE}
    --capture --rc lcov_branch_coverage=1 --initial --directory ./ --output-file
    app.base
  )

  add_custom_target(
    lcov
    # capture results
    COMMAND
      lcov --base-directory ./ --gcov-tool ${wexGCOV_USE} --capture --rc
      lcov_branch_coverage=1 --directory ./ --output-file app.run
    # combine tracefiles
    COMMAND
      lcov --gcov-tool ${wexGCOV_USE} --add-tracefile app.base --rc
      lcov_branch_coverage=1 --add-tracefile app.run --output-file app.run
    # remove output that we are not interested in
    COMMAND
      lcov --gcov-tool ${wexGCOV_USE} --remove app.run "*boost/*" "*/usr/*"
      "*sample*" "*external/*" "*/test/*" "*factory/grid*"
      "*include/wex/factory/text-window.h" --rc lcov_branch_coverage=1
      --output-file app.run
    # generate local files
    COMMAND
      genhtml
      # coverage rate limits for line coverage
      --ignore-errors mismatch --rc genhtml_hi_limit=50 --rc
      genhtml_med_limit=35
      # coverage rate limits for function coverage
      --rc genhtml_function_hi_limit=80 --rc genhtml_function_med_limit=50
      --branch-coverage --title "${PROJECT_NAME}" app.run
    COMMENT "Running lcov"
    VERBATIM
  )
endif()

if(wexBUILD_TIDY)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

add_subdirectory(core)
add_subdirectory(test)
add_subdirectory(factory)
add_subdirectory(syntax)
add_subdirectory(data)
add_subdirectory(common)
add_subdirectory(ui)
add_subdirectory(ex)
add_subdirectory(stc)
add_subdirectory(vcs)
add_subdirectory(del)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND MSVC)
  set_target_properties(
    ${wex_own_LIBRARIES}
    PROPERTIES
      VERSION ${PROJECT_VERSION}
      SOVERSION ${PROJECT_VERSION_MAJOR}
      DEBUG_POSTFIX "d"
  )
else()
  set_target_properties(
    ${wex_own_LIBRARIES}
    PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()
