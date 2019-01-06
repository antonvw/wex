project(wex-lib)

add_subdirectory(src)
add_subdirectory(src/report)

if (wexBUILD_SAMPLES)
  add_subdirectory(sample)
endif ()

if (wexBUILD_TESTS)
  add_subdirectory(test)
endif ()

add_subdirectory(locale)

if (CMAKE_BUILD_TYPE MATCHES "Coverage")
  add_custom_target(lcov-prep
    # prepare initial tracefile
    lcov --base-directory ./ --gcov-tool $GCOV --capture --initial --directory ./ --output-file app.base)
    
  add_custom_target(lcov
    # capture results
    COMMAND lcov --base-directory ./ --gcov-tool $GCOV --capture --directory ./ --output-file app.run
    # combine tracefiles
    COMMAND lcov --gcov-tool $GCOV --add-tracefile app.base --add-tracefile app.run --output-file app.run
    # remove output that we are not interested in
    COMMAND lcov --gcov-tool $GCOV --remove app.run "/usr/*" "*sample*" "*external/*" --output-file app.run
    # generate local files
    COMMAND genhtml --no-branch-coverage --title "${PROJECT_NAME}" app.run
    COMMENT "Running lcov" VERBATIM)    
  
  add_custom_target(lcov-coveralls
    # update coveralls
    COMMAND coveralls-lcov app.run
    # cleanup
    COMMAND rm -f app.base app.run)
endif ()
