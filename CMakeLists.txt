project(wxExtension)

if (WIN32)
  cmake_minimum_required(VERSION 3.0.0)
else()
  cmake_minimum_required(VERSION 2.8.7)
endif()

function(process_po_files)
  if (GETTEXT_FOUND)
    file(GLOB files *.po)
    
    foreach(filename ${files})
    
      string(FIND ${filename} "-" pos1 REVERSE)
      string(FIND ${filename} "." pos2 REVERSE)
      
      math(EXPR pos1 "${pos1} + 1")
      math(EXPR len "${pos2} - ${pos1}")
      
      string(SUBSTRING ${filename} ${pos1} ${len} lang)
      
      gettext_process_po_files(${lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
        PO_FILES ${filename})
    
      if (wxWidgets_ROOT_DIR AND ${ARGC} GREATER 0)
        gettext_process_po_files(${lang} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
          PO_FILES ${wxWidgets_ROOT_DIR}/locale/${lang}.po)
      endif ()
      
    endforeach()
  endif()
endfunction()  

function(pack)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_PACKAGE_VERSION 3.1.0)
  set(CPACK_PACKAGE_CONTACT "Anton van Wezenbeek")
  set(CPACK_PACKAGE_VENDOR "Coffee Tigers")
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")
  
  if (WIN32)
    install(FILES C:/Windows/syswow64/msvcp120.dll C:/Windows/syswow64/msvcr120.dll DESTINATION ${CONFIG_INSTALL_DIR})
  endif()

  install(DIRECTORY extension/data/ DESTINATION ${CONFIG_INSTALL_DIR})

  include(CPack)
endfunction()  

# Find the ODBC driver manager includes and library.
# 
# ODBC is an open standard for connecting to different databases in a
# semi-vendor-independent fashion.  First you install the ODBC driver
# manager.  Then you need a driver for each separate database you want
# to connect to (unless a generic one works).  VTK includes neither
# the driver manager nor the vendor-specific drivers: you have to find
# those yourself.
#  
# This module defines
# ODBC_INCLUDE_DIR where to find sql.h
# ODBC_LIBRARIES, the libraries to link against to use ODBC
# ODBC_FOUND.  If false, you cannot build anything that requires MySQL.
# also defined, but not for general use is
# ODBC_LIBRARY, where to find the ODBC driver manager library.

set( ODBC_FOUND 0 )

#---For the windows platform ODBC is located automatically
if (WIN32)
  set(ODBC_INCLUDE_DIR "")
  set(ODBC_LIBRARY odbc32.lib) 
  set(ODBC_FOUND 1)
else ()
  find_path(ODBC_INCLUDE_DIR sqlext.h
    /usr/include
    /usr/include/odbc
    /usr/local/include
    /usr/local/include/odbc
    /usr/local/odbc/include
    $ENV{ODBC_DIR}/include
    DOC "Specify the directory containing sql.h."
  )

  find_library( ODBC_LIBRARY NAMES iodbc odbc odbc32
    PATHS
    /usr/lib
    /usr/lib/odbc
    /usr/local/lib
    /usr/local/lib/odbc
    /usr/local/odbc/lib
    $ENV{ODBC_DIR}/lib
    DOC "Specify the ODBC driver manager library here."
  )
  
  if (ODBC_LIBRARY AND ODBC_INCLUDE_DIR)
    set( ODBC_FOUND 1 )
  endif()
endif()

set(ODBC_LIBRARIES ${ODBC_LIBRARY})

mark_as_advanced( ODBC_FOUND ODBC_LIBRARY ODBC_EXTRA_LIBRARIES ODBC_INCLUDE_DIR )

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} 
    /D_CRT_SECURE_NO_WARNINGS 
    /D_SCL_SECURE_NO_WARNINGS /DwxExUSE_PORTABLE")

  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(wxWidgets_CONFIGURATION mswud)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  else ()
    set(wxWidgets_CONFIGURATION mswu)
  endif ()
else ()
  if (CMAKE_BUILD_TYPE MATCHES "Coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O0 --coverage")
  endif ()
  
  if (CMAKE_BUILD_TYPE MATCHES "Profile")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O0 -pg")
  endif ()
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")
endif ()

if (WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FindwxWidgets.cmake
      "C:/Program Files (x86)/CMake/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules/FindwxWidgets.cmake" COPYONLY)

  find_package(wxWidgets COMPONENTS scintilla aui stc html adv xml core base net REQUIRED)
else ()
  find_package(wxWidgets COMPONENTS           aui stc html adv xml core base net REQUIRED)
endif ()

option(wxExUSE_OTL "Oracle Template Library")
option(wxExUSE_SSS "Sync Socket Server")

if (${wxExUSE_OTL} MATCHES ON)
  add_definitions(-DwxExUSE_OTL)
endif ()

if (WIN32)
  set(CONFIG_INSTALL_DIR bin)
  set(LOCALE_INSTALL_DIR bin)
else ()
  set(CONFIG_INSTALL_DIR $ENV{HOME}/.${PROJECT_NAME})
  set(LOCALE_INSTALL_DIR share/locale/)
endif ()

include(${wxWidgets_USE_FILE})
include(FindGettext)

enable_testing()

add_subdirectory(extension)

include_directories(../extension/include)

add_subdirectory(syncped)

if (wxExUSE_OTL)
  add_subdirectory(syncodbcquery)
endif ()

if (wxExUSE_SSS)
  add_subdirectory(syncsocketserver)
endif ()

pack()
