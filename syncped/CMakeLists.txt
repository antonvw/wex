project(syncped)

include_directories(../extension/include)

file(GLOB SRCS "*.cpp")
  
if (WIN32)
  set(SRCS ${SRCS} app.rc)
endif()
  
if(NOT APPLE )
  add_executable(
    ${PROJECT_NAME}
    WIN32 
    ${SRCS})
else()
  # Define some settings for the Bundle
  set( MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_ICON_FILE app-large.icns )
  set( MACOSX_BUNDLE_INFO_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}, Copyright 2015  Anton van Wezenbeek" )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}" )
  set( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
  set( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
  set( MACOSX_BUNDLE_COPYRIGHT "(C) 2005-2008 Stewart Adam" )

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
 
target_link_libraries(
  ${PROJECT_NAME}
  wxex-rep
  wxex
  ${wxWidgets_LIBRARIES} 
  )

add_subdirectory(locale)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

if (NOT WIN32)
  install(FILES syncped.desktop DESTINATION /usr/share/applications)
  install(FILES syncped.png DESTINATION /usr/share/icons/hicolor/32x32/)
endif()

pack()
