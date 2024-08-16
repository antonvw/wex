project(wex-test-ui)

if(wexENABLE_ODBC)
  find_package(ODBC)

  if(ODBC_FOUND AND NOT MSVC)
    add_definitions(-DwexUSE_ODBC)

    include_directories(${ODBC_INCLUDE_DIR})
    include_directories(../../../external/otl)
  endif()
endif()

file(GLOB SRCS ${SRCS} "*.cpp")

wex_test_app("wex-ui")
