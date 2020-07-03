project(wex-lib)

add_subdirectory(core)
add_subdirectory(data)
add_subdirectory(common)
add_subdirectory(stc)
add_subdirectory(ui)
add_subdirectory(vi)
add_subdirectory(report)

if (wexBUILD_SAMPLES)
  add_subdirectory(sample)
endif ()
