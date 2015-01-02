project(syncodbcquery)
cmake_minimum_required(VERSION 2.8.7)

find_package(wxWidgets COMPONENTS scintilla aui stc html adv xml core base REQUIRED)

include(
  ${wxWidgets_USE_FILE})
  
include_directories(../extension/include)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif(WIN32)
  
add_executable(
  ${PROJECT_NAME}
  ${SRCS})
  
target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  )
