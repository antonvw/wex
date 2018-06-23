project(syncped)

include(../cmake/common.cmake)

file(GLOB SRCS "*.cpp")
  
if (WIN32)
  set(SRCS ${SRCS} app.rc)
endif()

if (NOT APPLE)
  add_executable(${PROJECT_NAME} WIN32 ${SRCS})
else()
  # Define some settings for the Bundle
  # add to mac osx Info.plist: <key>NSPrincipleClass</key> (new cmake version)
  set( MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_ICON_FILE app-large.icns )
  set( MACOSX_BUNDLE_INFO_STRING ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING ${EX_VERSION} )
  set( MACOSX_BUNDLE_BUNDLE_VERSION ${EX_VERSION} )
  set( MACOSX_BUNDLE_COPYRIGHT "(c) ${EX_COPYRIGHT} Anton van Wezenbeek" )

  # create a bundle with an icon too!
  add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${SRCS} )
  
  # Allows for bundle re-creation just by running "make". Also installs bundle icon
  add_custom_target( osx_bundle_dirs
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/MacOS
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../../${PROJECT_NAME}/${MACOSX_BUNDLE_ICON_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app/Contents/Resources/${MACOSX_BUNDLE_ICON_FILE}
   )
  add_dependencies( ${PROJECT_NAME} osx_bundle_dirs )
  # This tells cmake where to place files inside the bundle
  set_source_files_properties( ${ProjectName_RESOURCES} ${ProjectName_TRANSLATIONS}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
endif()

if (NOT (CMAKE_BUILD_TYPE MATCHES "Coverage"))
  cotire(${PROJECT_NAME})
endif ()
 
target_link_all()

add_subdirectory(locale)

if (NOT APPLE)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
else()
  install(TARGETS ${PROJECT_NAME} DESTINATION /Applications)
endif()

if (NOT WIN32)
  if (EXISTS "/usr/share/applications")
    install(FILES ${PROJECT_NAME}.desktop DESTINATION /usr/share/applications)
  endif()
  if (EXISTS "/usr/share/icons")
    install(FILES ${PROJECT_NAME}.png DESTINATION /usr/share/icons/hicolor/scalable/apps/)
    install(FILES ${PROJECT_NAME}.svg DESTINATION /usr/share/icons/hicolor/scalable)
  endif()
endif()

pack()
