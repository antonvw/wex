if(wexBUILD_TIDY)
  file(GLOB SRCS "*.cpp")
else()
  file(
    GLOB SRCS
    "*.cpp"
    "../../external/wxMaterialDesignArtProvider/MaterialDesign/*.cpp"
  )
endif()

if(wexENABLE_ODBC)
  find_package(ODBC)

  if(ODBC_FOUND AND NOT MSVC)
    add_definitions(-DwexUSE_ODBC)

    include_directories(${ODBC_INCLUDE_DIR})
    include_directories(../../external/otl)
  endif()
endif()

add_library(wex-ui ${SRCS})

add_subdirectory(ctags)
