find_package(Python COMPONENTS Development Interpreter)

if (NOT Python_FOUND)
  return()
endif()

find_package(SWIG 4.1 COMPONENTS python)
include(UseSWIG)

if (NOT SWIG_FOUND)
  return()
endif()

include_directories(${CMAKE_SOURCE_DIR}/bindings ${Python_INCLUDE_DIRS})

# see wex.i
set(CORE-SRCS 
  ${CMAKE_SOURCE_DIR}/src/core/chrono.cpp
  ${CMAKE_SOURCE_DIR}/src/core/interruptible.cpp)

set_property(SOURCE wex.i PROPERTY CPLUSPLUS ON)

swig_add_library(${PROJECT_NAME} 
  TYPE SHARED 
  LANGUAGE python 
  SOURCES wex.i ${CORE-SRCS})
  
if (APPLE)
  set(SWIG_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/wex.py
    ${CMAKE_CURRENT_BINARY_DIR}/_wex.dylib)

  install(FILES ${SWIG_FILES}
    DESTINATION ${Python_SITELIB})
endif ()
