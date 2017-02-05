project(syncsocketserver)

include_directories(../extension/include)
include_directories(../extension/pugixml/src)

FILE(GLOB SRCS "*.cpp")
  
if(WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
add_executable(
  ${PROJECT_NAME}
  WIN32 
  ${SRCS})

target_link_all()

add_subdirectory(locale)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

pack()
