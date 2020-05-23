function(pack)
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
      # c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT
      file(GLOB_RECURSE dlls 
        "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC/redist/x86/*.dll")
    else()
      # Visual studio 2019:
      # C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.23.27820\x86\Microsoft.VC142.CRT
      file(GLOB_RECURSE dlls 
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Redist/MSVC/14*/x86/*.dll")
    endif()

    install(FILES ${dlls} DESTINATION ${CONFIG_INSTALL_DIR})
  endif()

  configure_file(../../data/wex-conf.elp.cmake conf.elp)

  install(DIRECTORY ../data/ DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xml" )
  
  install(DIRECTORY ../data/ DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.xsl" )
  
  install(DIRECTORY ../data/ DESTINATION ${CONFIG_INSTALL_DIR} 
    FILES_MATCHING PATTERN "*.txt" )
  
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/conf.elp DESTINATION ${CONFIG_INSTALL_DIR})
          
  if (NOT WIN32)
    install(CODE "EXECUTE_PROCESS(COMMAND chown -R ${user} ${CONFIG_INSTALL_DIR})")
  endif()

  include(CPack)
endfunction()  

function(process_po_files)
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
          
        gettext_process_po_files(${locale} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
          PO_FILES ${filename})

        if (${ARGC} GREATER 0)
          set(wxWidgets_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/wxWidgets)
          gettext_process_po_files(${locale} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
            PO_FILES ${wxWidgets_ROOT_DIR}/locale/${lang}.po)
        endif ()
      
      endforeach()
  endif()
endfunction()  

macro(target_link_all)
  set (extra_macro_args ${ARGN})
  set (wxWidgets_LIBRARIES aui adv stc html core net base)
  set (wex_LIBRARIES 
    wex-report wex-common wex-data wex-lexer 
    wex-stc wex-ui wex-vcs wex-vi wex-core wex-lexer wex-common wex-ui wex-data)
          
  if (WIN32)
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_LIBRARIES}
      ${wxWidgets_LIBRARIES} wxscintilla
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
      c++fs
      )
  else ()
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_LIBRARIES}
      ${wxWidgets_LIBRARIES} 
      ${Boost_LIBRARIES}
      ${extra_macro_args}
      stdc++
      stdc++fs
#      /usr/gnat/lib64/libstdc++.a
#      /usr/gnat/lib64/libstdc++fs.a
      m
      )
  endif ()
endmacro()  

function(test_app)
  add_executable(
    ${PROJECT_NAME} 
    ${SRCS})

  if (ODBC_FOUND)
    target_link_all(${ODBC_LIBRARIES})
  else ()
    target_link_all()
  endif()
  
  add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src)
endfunction()
          
if (WIN32)
  set(LOCALE_INSTALL_DIR bin)
else ()
  set(LOCALE_INSTALL_DIR share/locale/)
endif ()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS \
    /std:c++17 /Zc:__cplusplus")

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
    -std=c++17 -Wno-overloaded-virtual -Wno-reorder -Wno-write-strings \
    -Wno-deprecated-declarations -Wno-unused-result")
endif ()

file(GLOB_RECURSE wexSETUP_H ${wex_BINARY_DIR}/*setup.h)
# use only first element from list
list(GET wexSETUP_H 0 wexSETUP_H) 
get_filename_component(wexSETUP_H ${wexSETUP_H} DIRECTORY)
get_filename_component(wexSETUP_H ${wexSETUP_H} DIRECTORY)

list(APPEND wxTOOLKIT_INCLUDE_DIRS 
  ${wexSETUP_H}
  src/include 
  external/json/single_include 
  external/wxWidgets/include 
  external/ctags/libreadtags 
  external/easyloggingpp/src 
  external/pugixml/src)

foreach(arg ${wxTOOLKIT_INCLUDE_DIRS})
  include_directories(${arg})
endforeach ()

list(APPEND wxTOOLKIT_DEFINITIONS HAVE_WCSLEN)

foreach(arg ${wxTOOLKIT_DEFINITIONS})
  add_definitions(-D${arg})
endforeach ()
