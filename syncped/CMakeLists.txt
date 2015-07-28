project(syncped)

include_directories(../extension/include)

file(GLOB SRCS "*.cpp")
  
if (WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})
  
target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  )

add_subdirectory(locale)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
#install(TARGETS ${PROJECT_NAME}.manifest DESTINATION bin)

pack()
