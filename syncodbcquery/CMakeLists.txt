project(syncodbcquery)
cmake_minimum_required(VERSION 2.8.7)

if (NOT ${ODBC_FOUND})
  message(FATAL_ERROR "odbc lib not found")
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /DwxExUSE_PORTABLE")
  
  if (CMAKE_BUILD_TYPE EQUAL "Debug")
    set(wxWidgets_CONFIGURATION mswud)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  else (CMAKE_BUILD_TYPE EQUAL "Debug")
    set(wxWidgets_CONFIGURATION mswu)
  endif (CMAKE_BUILD_TYPE EQUAL "Debug")
else (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")
endif()

if (WIN32)
  find_package(wxWidgets COMPONENTS scintilla aui stc html adv xml core base REQUIRED)
else (WIN32)
  find_package(wxWidgets COMPONENTS           aui stc html adv xml core base REQUIRED)
endif ()

include(
  ${wxWidgets_USE_FILE})
  
include_directories(../extension/include)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif(WIN32)
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})
  
target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  ${ODBC_LIBRARIES}
  )
