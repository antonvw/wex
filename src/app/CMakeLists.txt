include(../../cmake/common.cmake)

if (wexBUILD_SP)
  add_subdirectory(syncped)
endif ()

if (wexBUILD_SOQ)
  add_subdirectory(syncodbcquery)
endif ()

if (wexBUILD_SSS)
  add_subdirectory(syncsocketserver)
endif ()
