SET(CMAKE_INSTALL_PREFIX ..)

add_subdirectory(src)
add_subdirectory(src/report)
add_subdirectory(sample)
add_subdirectory(test)

if (CMAKE_BUILD_TYPE MATCHES "Coverage")
  add_custom_target(lcov-prep
    lcov --base-directory ./ --capture --initial --directory ./ --output-file app.base)
    
  add_custom_target(lcov
    COMMAND lcov --base-directory ./ --capture --directory ./ --output-file app.run
    COMMAND lcov --add-tracefile app.base --add-tracefile app.run --output-file app.total
    # remove output for external and test libraries
    COMMAND lcov --remove app.total "/usr*" --output-file app.total
    COMMAND lcov --remove app.total "sample*" --output-file app.total
    COMMAND lcov --remove app.total "*wxExtension/sync*" --output-file app.total
    # when manually updating coveralls
    COMMAND coveralls-lcov --repo-token sOcBNDSlOjLSH3vs636zytnjuQtRWKLGP app.total
    COMMAND genhtml --no-branch-coverage --title "wxExtension Library" app.total
    COMMAND rm -f app.base app.run app.total
    COMMENT "Running lcov" VERBATIM)    
endif ()
