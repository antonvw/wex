file(GLOB_RECURSE wexSETUP_H ${CMAKE_BINARY_DIR}/*setup.h)
# use only first element from list
list(GET wexSETUP_H 0 wexSETUP_H) 

# functions

function(wex_config)
  if (WIN32)
    set(CONFIG_INSTALL_DIR bin)
  elseif (APPLE)
    set(CONFIG_INSTALL_DIR /Users/$ENV{USER}/.config/${PROJECT_NAME})
  else ()
    set(CONFIG_INSTALL_DIR /home/${user}/.config/${PROJECT_NAME})
  endif ()

  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_VERSION "${WEX_VERSION}")
  
  # See appveyor.yml (artifact).
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")

  if (MSVC)
    if (${MSVC_TOOLSET_VERSION} LESS 142)
      # Visual studio 2017:
      file(GLOB_RECURSE dlls 
        "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x86/*.dll")
    else()
      # Visual studio 2019:
      file(GLOB_RECURSE dlls 
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/vcruntime14*.dll")
    endif()

    install(FILES ${dlls} DESTINATION ${CONFIG_INSTALL_DIR})
  endif()

  # install config files in ${CONFIG_INSTALL_DIR}
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xml" )
  
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xsl" )
  
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/ 
    DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.txt" )

  if (NOT WIN32)
    install(CODE "EXECUTE_PROCESS(COMMAND chown -R ${user} ${CONFIG_INSTALL_DIR})")
  endif()
endfunction()  

function(wex_install)
  set(MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)

  # install FindWEX.cmake
  install(FILES ${CMAKE_SOURCE_DIR}/cmake/FindWEX.cmake 
    DESTINATION ${MODULE_INSTALL_DIR})
  
  # install some wxWidgets cmake files
  install(FILES ${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules/FindICONV.cmake
    DESTINATION ${MODULE_INSTALL_DIR})

  # install include files
  # this should be the same dir as in FindWEX.cmake
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/wex 
    DESTINATION "include/wex")

  install(DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/wx 
    DESTINATION "include/wex")

  install(FILES ${CMAKE_SOURCE_DIR}/external/pugixml/src/pugiconfig.hpp 
    DESTINATION "include/wex")

  install(FILES ${CMAKE_SOURCE_DIR}/external/pugixml/src/pugixml.hpp 
    DESTINATION "include/wex")

  install(FILES ${CMAKE_SOURCE_DIR}/external/ctags/libreadtags/readtags.h
    DESTINATION "include/wex")

  if (ODBC_FOUND)
    install(FILES ${CMAKE_SOURCE_DIR}/external/otl/otlv4.h
      DESTINATION "include/wex")
  endif ()
  
  install(FILES ${wexSETUP_H} 
    DESTINATION "include/wex/wx")
  
  # install libraries
  # this should be the same dir as in FindWEX.cmake
  if (MSVC)
    file(GLOB_RECURSE wex_LIBS ${CMAKE_BINARY_DIR}/*.lib)
  else ()
    if (wexBUILD_SHARED)
      if (APPLE)
        file(GLOB_RECURSE wex_LIBS ${CMAKE_BINARY_DIR}/*.dylib)
      else ()
        file(GLOB_RECURSE wex_LIBS ${CMAKE_BINARY_DIR}/*.so)
      endif ()
    else ()
      file(GLOB_RECURSE wex_LIBS ${CMAKE_BINARY_DIR}/*.a)
    endif ()
  endif ()
  
  install(FILES ${wex_LIBS} 
    DESTINATION "lib")
endfunction()

function(wex_process_po_files)
  # travis has problem with gettext
  if (GETTEXT_FOUND AND NOT DEFINED ENV{TRAVIS})
      file(GLOB files *.po)
      
      foreach(filename ${files})
        string(FIND ${filename} "-" pos1 REVERSE)
        string(FIND ${filename} "." pos2 REVERSE)
        
        math(EXPR pos1 "${pos1} + 1")
        math(EXPR len "${pos2} - ${pos1}")
        
        string(SUBSTRING ${filename} ${pos1} ${len} lang)
    
        set(locale ${lang})
    
        if (${locale} MATCHES "nl")
          set(locale "nl_NL")
        endif ()
    
        if (${locale} MATCHES "fr")
          set(locale "fr_FR")
        endif ()
          
        gettext_process_po_files(${locale} ALL 
          INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
          PO_FILES ${filename})

        set(wxWidgets_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/wxWidgets)
        gettext_process_po_files(${locale} ALL 
          INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
          PO_FILES ${wxWidgets_ROOT_DIR}/locale/${lang}.po)
      
      endforeach()
  endif()
endfunction()  

macro(wex_target_link_all)
  set (extra_macro_args ${ARGN})

  if (CENTOS)
    set (cpp_std_LIBRARIES 
      /usr/gnat/lib64/libstdc++.a
      /usr/gnat/lib64/libstdc++fs.a)
  else ()
    set (cpp_std_LIBRARIES 
      X11
      pthread
      stdc++
      stdc++fs)
  endif ()

  set (wxWidgets_LIBRARIES wxaui wxstc wxhtml wxcore wxnet wxbase wxscintilla)
  set (wex_LIBRARIES wex-del wex-stc wex-vi wex-ui wex-common wex-data wex-factory wex-core)
          
  if (WIN32)
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_LIBRARIES}
      ${wxWidgets_LIBRARIES}
      ${Boost_LIBRARIES}
      ${extra_macro_args}
      )
  elseif (APPLE)
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_LIBRARIES}
      ${wxWidgets_LIBRARIES} 
      ${Boost_LIBRARIES}
      ${extra_macro_args}
      stdc++
      )
  else ()
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_LIBRARIES}
      ${wxWidgets_LIBRARIES} 
      ${Boost_LIBRARIES}
      ${extra_macro_args}
      ${cpp_std_LIBRARIES}
      m
      )
  endif ()
endmacro()  

function(wex_test_app)
  add_executable(
    ${PROJECT_NAME} 
    ${SRCS})

  if (ODBC_FOUND)
    wex_target_link_all(${ODBC_LIBRARIES})
  else ()
    wex_target_link_all()
  endif()
  
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
endfunction()
          
# general setup
    
if (WIN32)
  set(LOCALE_INSTALL_DIR bin)
else ()
  set(LOCALE_INSTALL_DIR share/locale/)
endif ()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE
    /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS \
    /std:c++latest /Zc:__cplusplus")

  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  endif ()
else ()
  if (CMAKE_BUILD_TYPE MATCHES "Coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIC \
      --param ggc-min-expand=3 --param ggc-min-heapsize=5120")
  endif ()
  
  if (CMAKE_BUILD_TYPE MATCHES "Profile")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  endif ()
  
  if (CMAKE_BUILD_TYPE MATCHES "valgrind")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  endif ()
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -std=c++2a -Wno-overloaded-virtual -Wno-reorder -Wno-write-strings \
    -Wno-deprecated-declarations -Wno-unused-result")
endif ()

get_filename_component(wexSETUP_DIR_H ${wexSETUP_H} DIRECTORY)
get_filename_component(wexSETUP_DIR_H ${wexSETUP_DIR_H} DIRECTORY)

list(APPEND wxTOOLKIT_INCLUDE_DIRS 
  ${wexSETUP_DIR_H}
  include 
  external/json/single_include
  external/pugixml/src
  external/ctags/libreadtags/
  external/wxWidgets/include
  external)

foreach(arg ${wxTOOLKIT_INCLUDE_DIRS})
  include_directories(${arg})
endforeach ()

list(APPEND wxTOOLKIT_DEFINITIONS HAVE_WCSLEN)

foreach(arg ${wxTOOLKIT_DEFINITIONS})
  add_definitions(-D${arg})
endforeach ()
