project(syncped)
cmake_minimum_required(VERSION 2.8.7)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(wxWidgets REQUIRED xml aui stc html adv core base)

option(wxExUSE_OTL 
  "Use tutorial provided math implementation" OFF) 
        
option(wxExUSE_PORTABLE 
  "Use tutorial provided math implementation" OFF) 
        
include(
  ${wxWidgets_USE_FILE})

set(SRCS 
  app.cpp 
  frame.cpp
  support.cpp)
  
include_directories(
  ../extension/include/)

find_library(WXEX
    NAMES wxex
    HINTS "../extension/src"
  )

find_library(WXEX-REP
    NAMES wxex-rep 
    HINTS "../extension/src/report"
  )

add_executable(
  ${PROJECT_NAME}
  ${SRCS})
  
target_link_libraries(
  ${PROJECT_NAME}
  ${WXEX-REP}
  ${WXEX}
  ${wxWidgets_LIBRARIES} 
  )
