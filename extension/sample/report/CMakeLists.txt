include(
  ${wxWidgets_USE_FILE})

set(SRCS 
  app.cpp 
  )
  
include_directories(
  ../../include/)

add_executable(
  wxExSampleReport 
  ${SRCS})

target_link_libraries(
  wxExSampleReport 
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  )
