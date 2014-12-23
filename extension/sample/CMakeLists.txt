include(
  ${wxWidgets_USE_FILE})

set(SRCS 
  app.cpp 
  )
  
include_directories(
  ../include/)

add_executable(
  wxExSample 
  ${SRCS})
  
target_link_libraries(
  wxExSample 
  wxex
  ${wxWidgets_LIBRARIES} 
  )

add_subdirectory(report)
