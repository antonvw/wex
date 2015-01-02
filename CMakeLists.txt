cmake_minimum_required(VERSION 2.8.7)

add_subdirectory(extension)

include_directories(../extension/include)

if (wxExUSE_OTL)
  add_subdirectory(syncodbcquery)
endif (wxExUSE_OTL)

add_subdirectory(syncped)
add_subdirectory(syncsocketserver)
