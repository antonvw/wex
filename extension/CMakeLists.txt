project(wxExtension)

cmake_minimum_required(VERSION 2.8.7)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(wxWidgets REQUIRED xml aui stc html adv core base)

option(wxExUSE_OTL 
  "Use tutorial provided math implementation" OFF) 
        
option(wxExUSE_PORTABLE 
  "Use tutorial provided math implementation" OFF) 
        
include(
  ${wxWidgets_USE_FILE})

add_subdirectory(src)
add_subdirectory(src/report)
add_subdirectory(sample)
add_subdirectory(test)
