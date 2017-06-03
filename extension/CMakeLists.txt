project(wxExtensionLibrary)

include_directories(include)
include_directories(pugixml/src)
include_directories(tclap/include)

add_subdirectory(src)
add_subdirectory(src/report)
add_subdirectory(sample)
add_subdirectory(test)
add_subdirectory(locale)

if (CMAKE_BUILD_TYPE MATCHES "Coverage")
  add_custom_target(lcov-prep
    # prepare initial tracefile
    lcov --base-directory ./ --capture --initial --directory ./ --output-file app.base)
    
  add_custom_target(lcov
    # capture results
    COMMAND lcov --base-directory ./ --capture --directory ./ --output-file app.run
    # combine tracefiles
    COMMAND lcov --add-tracefile app.base --add-tracefile app.run --output-file app.run
    # remove output that we are not interested in
    COMMAND lcov --remove app.run "/usr/*" "*sample*" "*eval.hpp" "*otlv4.h*" "*tclap*"
      "*doctest*" "readtags.*" "*pugi*" --output-file app.run
    # generate local files
    COMMAND genhtml --no-branch-coverage --title "${PROJECT_NAME}" app.run
    COMMENT "Running lcov" VERBATIM)    
  
  add_custom_target(lcov-coveralls
    # update coveralls
    COMMAND coveralls-lcov --repo-token ${COVERALLS_TOKEN} app.run
    # cleanup
    COMMAND rm -f app.base app.run)
endif ()
