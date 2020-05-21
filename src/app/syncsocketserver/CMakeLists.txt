project(syncsocketserver)

file(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})

target_link_all()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

pack()
