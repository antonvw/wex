function(pack)
  if (WIN32)
    set(CONFIG_INSTALL_DIR bin)
  else ()
    set(CONFIG_INSTALL_DIR /home/${user}/.config/${PROJECT_NAME})
  endif ()

  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_VERSION "${EX_VERSION}")
  set(CPACK_PACKAGING_INSTALL_PREFIX ".")
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")

  if (MSVC)
    if (MSVC15)
      set(msvc "14") # use v141 MSVC toolset
    elseif (MSVC14)
      set(msvc "14")
    elseif (MSVC12)
      set(msvc "12")
    elseif (MSVC11)
      set(msvc "11")
    elseif (MSVC10)
      set(msvc "10")
    endif()

    file(GLOB dlls "C:/Program Files (x86)/Microsoft Visual Studio ${msvc}.0/VC/redist/x86/Microsoft.VC${msvc}0.CRT/*.dll")
    install(FILES ${dlls} DESTINATION ${CONFIG_INSTALL_DIR})
  endif()

  configure_file(../extension/data/conf.elp ${CONFIG_INSTALL_DIR}/conf.elp)
  install(DIRECTORY ../extension/data/ DESTINATION ${CONFIG_INSTALL_DIR} 
    USE_SOURCE_PERMISSIONS
    FILES_MATCHING PATTERN "*.elp" EXCLUDE )
  
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
          
        if (NOT WIN32)
          # TODO: this should be part of cmake,
          # however wxWidgets_ROOT_DIR is empty for linux, so set it here,
          # search at several places...
          if (EXISTS "$ENV{HOME}/wxWidgets")
            set(wxWidgets_ROOT_DIR $ENV{HOME}/wxWidgets)
          endif ()
        endif ()
        
        gettext_process_po_files(${locale} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
          PO_FILES ${filename})

        if (wxWidgets_ROOT_DIR AND ${ARGC} GREATER 0)
          gettext_process_po_files(${locale} ALL INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
            PO_FILES ${wxWidgets_ROOT_DIR}/locale/${lang}.po)
        endif ()
      
      endforeach()
  endif()
endfunction()  

macro(target_link_all)
  set (extra_macro_args ${ARGN})
  set (wxWidgets_LIBRARIES aui stc html adv core base net)
          
if (WIN32)
  target_link_libraries(
    ${PROJECT_NAME}
    wxex-rep
    wxex
    ${wxWidgets_LIBRARIES} wxscintilla
    ${extra_macro_args}
    )
else ()
  target_link_libraries(
    ${PROJECT_NAME}
    wxex-rep
    wxex
    ${wxWidgets_LIBRARIES} 
    ${extra_macro_args}
    stdc++fs
    )
endif ()

  if (NOT (CMAKE_BUILD_TYPE MATCHES "Coverage"))
    cotire(${PROJECT_NAME})
  endif()
endmacro()  

if (WIN32)
  set(LOCALE_INSTALL_DIR bin)
else ()
  set(LOCALE_INSTALL_DIR share/locale/)
endif ()

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /DCRT_SECURE_NO_DEPRECATE /std:c++latest")

  if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  endif ()
else ()
  if (CMAKE_BUILD_TYPE MATCHES "Coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fPIC --param ggc-min-expand=3 --param ggc-min-heapsize=5120")
  endif ()
  
  if (CMAKE_BUILD_TYPE MATCHES "Profile")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  endif ()
  
  if (CMAKE_BUILD_TYPE MATCHES "valgrind")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  endif ()
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-overloaded-virtual -Wno-reorder -Wno-write-strings -Wno-deprecated-declarations -Wno-unused-result")
endif ()

file(GLOB_RECURSE wxExSETUP_H ${wxExtension_BINARY_DIR}/*.h)
# use only first element from list
list(GET wxExSETUP_H 0 wxExSETUP_H) 
get_filename_component(wxExSETUP_H ${wxExSETUP_H} DIRECTORY)
get_filename_component(wxExSETUP_H ${wxExSETUP_H} DIRECTORY)

list(APPEND wxTOOLKIT_INCLUDE_DIRS 
  ${wxExSETUP_H}
  external/wxWidgets/include 
  extension/include 
  external/ctags external/easyloggingpp/src 
  external/fsm external/pugixml/src 
  external/shunting-yard external/tclap/include)

foreach(arg ${wxTOOLKIT_INCLUDE_DIRS})
  include_directories(${arg})
endforeach ()

list(APPEND wxTOOLKIT_DEFINITIONS HAVE_WCSLEN)

foreach(arg ${wxTOOLKIT_DEFINITIONS})
  add_definitions(-D${arg})
endforeach ()
