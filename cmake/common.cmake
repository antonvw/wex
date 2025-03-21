file(GLOB_RECURSE wexSETUP_H ${CMAKE_BINARY_DIR}/*setup.h)
# use only first element from list
list(GET wexSETUP_H 0 wexSETUP_H)

# functions

function(wex_config)
  if(WIN32)
    set(CONFIG_INSTALL_DIR bin)
  elseif(APPLE)
    set(CONFIG_INSTALL_DIR /Users/$ENV{USER}/.config/${PROJECT_NAME})
  else()
    set(CONFIG_INSTALL_DIR /home/${user}/.config/${PROJECT_NAME})
  endif()

  set(CPACK_GENERATOR "ZIP")
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_VERSION "${WEX_VERSION}")

  # For artifacts, not yet used.
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}")

  if(MSVC)
    if(${MSVC_TOOLSET_VERSION} LESS 143)
      # Visual studio 2019:
      file(
        GLOB_RECURSE dlls
        "C:/Program Files (x86)/Microsoft Visual Studio/2019/vcruntime14*.dll"
      )
    else()
      # Visual studio 2022:
      file(
        GLOB_RECURSE dlls
        "C:/Program Files/Microsoft Visual Studio/2022/vcruntime14*.dll"
      )
    endif()

    install(FILES ${dlls} DESTINATION ${CONFIG_INSTALL_DIR})
  endif()

  # install config files in ${CONFIG_INSTALL_DIR}
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/data/
    DESTINATION ${CONFIG_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.xml"
  )

  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/data/
    DESTINATION ${CONFIG_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.xsl"
  )

  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/data/
    DESTINATION ${CONFIG_INSTALL_DIR}
    FILES_MATCHING
    PATTERN "*.txt"
  )

  if(NOT WIN32)
    install(
      CODE "EXECUTE_PROCESS(COMMAND chown -R ${user} ${CONFIG_INSTALL_DIR})"
    )
  endif()
endfunction()

function(wex_install)
  set(WEX_INSTALL_DIR "include/wex/${WEX_VERSION_INCLUDE}")
  set(MODULE_INSTALL_DIR ${CMAKE_ROOT}/Modules)

  # install Findwex.cmake
  install(
    FILES ${CMAKE_SOURCE_DIR}/cmake/Findwex.cmake
    DESTINATION ${MODULE_INSTALL_DIR}
  )

  # install some wxWidgets cmake files
  install(
    FILES
      ${CMAKE_SOURCE_DIR}/external/wxWidgets/build/cmake/modules/FindICONV.cmake
    DESTINATION ${MODULE_INSTALL_DIR}
  )

  # install include files
  # this should be the same dir as in Findwex.cmake
  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/wex
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/external/wxWidgets/include/wx
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    FILES ${CMAKE_SOURCE_DIR}/external/pugixml/src/pugiconfig.hpp
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    FILES ${CMAKE_SOURCE_DIR}/external/pugixml/src/pugixml.hpp
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    FILES ${CMAKE_SOURCE_DIR}/external/ctags/libreadtags/readtags.h
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    FILES ${CMAKE_SOURCE_DIR}/external/doctest/doctest/doctest.h
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    DIRECTORY
      ${CMAKE_SOURCE_DIR}/external/wxMaterialDesignArtProvider/MaterialDesign/
    DESTINATION ${WEX_INSTALL_DIR}
  )

  install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/external/trompeloeil/include/
    DESTINATION ${WEX_INSTALL_DIR}
  )

  if(ODBC_FOUND)
    install(
      FILES ${CMAKE_SOURCE_DIR}/external/otl/otlv4.h
      DESTINATION ${WEX_INSTALL_DIR}
    )
  endif()

  install(FILES ${wexSETUP_H} DESTINATION ${WEX_INSTALL_DIR}/wx)

  # install libraries
  # this should be the same dir as in Findwex.cmake
  if(MSVC)
    file(GLOB_RECURSE wex_own_LIBRARIES ${CMAKE_BINARY_DIR}/*.lib)
  else()
    if(wexBUILD_SHARED)
      if(APPLE)
        file(
          GLOB_RECURSE wex_own_LIBRARIES
          ${CMAKE_BINARY_DIR}/*.dylib
          ${CMAKE_BINARY_DIR}/*.a
        )
      else()
        file(
          GLOB_RECURSE wex_own_LIBRARIES
          ${CMAKE_BINARY_DIR}/*.so*
          ${CMAKE_BINARY_DIR}/*.a
        )
      endif()
    else()
      file(GLOB_RECURSE wex_own_LIBRARIES ${CMAKE_BINARY_DIR}/*.a)
    endif()
  endif()

  # install wex-keywords.resource
  if(wexBUILD_SAMPLES AND wexBUILD_TESTS)
    install(
      FILES ${CMAKE_SOURCE_DIR}/test/app/wex-keywords.resource
      DESTINATION "lib"
    )
  endif()

  install(FILES ${wex_own_LIBRARIES} DESTINATION "lib")

  if(WIN32)
    install(
      FILES ${CMAKE_SOURCE_DIR}/build-gen.ps1
      DESTINATION bin
      RENAME wex-build-gen.ps1
      PERMISSIONS WORLD_EXECUTE WORLD_WRITE WORLD_READ
    )
  else()
    install(
      FILES ${CMAKE_SOURCE_DIR}/build-gen.sh
      DESTINATION bin
      RENAME wex-build-gen.sh
      PERMISSIONS WORLD_EXECUTE WORLD_WRITE WORLD_READ
    )
  endif()
endfunction()

function(wex_process_po_files)
  if(GETTEXT_FOUND)
    file(GLOB files *.po)

    foreach(filename ${files})
      string(FIND ${filename} "-" pos1 REVERSE)
      string(FIND ${filename} "." pos2 REVERSE)

      math(EXPR pos1 "${pos1} + 1")
      math(EXPR len "${pos2} - ${pos1}")

      string(SUBSTRING ${filename} ${pos1} ${len} lang)

      set(locale ${lang})

      if(${locale} MATCHES "nl")
        set(locale "nl_NL")
      endif()

      if(${locale} MATCHES "fr")
        set(locale "fr_FR")
      endif()

      gettext_process_po_files(
        ${locale}
        ALL
        INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
        PO_FILES ${filename}
      )

      set(wxWidgets_ROOT_DIR ${CMAKE_SOURCE_DIR}/external/wxWidgets)
      gettext_process_po_files(
        ${locale}
        ALL
        INSTALL_DESTINATION ${LOCALE_INSTALL_DIR}
        PO_FILES ${wxWidgets_ROOT_DIR}/locale/${lang}.po
      )
    endforeach()
  endif()
endfunction()

function(wex_target_link_all)
  if(${ARGC} STREQUAL "0")
    set(wex_use_LIBRARIES ${wex_own_LIBRARIES} ${ODBC_LIBRARIES})
  else()
    set(wex_use_LIBRARIES ${ARGN})
  endif()

  separate_arguments(wex_use_LIBRARIES)

  if(CENTOS)
    set(
      cpp_std_LIBRARIES
      /usr/gnat/lib64/libstdc++.a
      /usr/gnat/lib64/libstdc++fs.a
    )
  else()
    set(cpp_std_LIBRARIES X11 pthread stdc++ stdc++fs)
  endif()

  set(
    wxWidgets_LIBRARIES
    wxaui
    wxstc
    wxhtml
    wxcore
    wxnet
    wxbase
    wxscintilla
  )

  if(WIN32)
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_use_LIBRARIES}
      ${wxWidgets_LIBRARIES}
      ${Boost_LIBRARIES}
    )
  elseif(APPLE)
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_use_LIBRARIES}
      ${wxWidgets_LIBRARIES}
      ${Boost_LIBRARIES}
      stdc++
    )
  else()
    target_link_libraries(
      ${PROJECT_NAME}
      ${wex_use_LIBRARIES}
      ${wxWidgets_LIBRARIES}
      ${Boost_LIBRARIES}
      ${cpp_std_LIBRARIES}
      m
    )
  endif()
endfunction()

if(APPLE)
  set_property(GLOBAL PROPERTY test_libs wex-test)
else()
  set_property(GLOBAL PROPERTY wex-test test_libs)
endif()

function(add_test_libs)
  get_property(tmp GLOBAL PROPERTY test_libs)
  foreach(arg ${ARGV})
    set(tmp "${tmp} ${arg}")
  endforeach()

  if(APPLE)
    set_property(GLOBAL PROPERTY test_libs "${tmp}")
  else()
    set_property(GLOBAL PROPERTY "${tmp}" test_libs)
  endif()
endfunction(add_test_libs)

function(wex_test_app libs)
  add_test_libs(${libs})
  add_executable(${PROJECT_NAME} ${SRCS})

  get_property(tmp GLOBAL PROPERTY test_libs)

  if(ODBC_FOUND)
    wex_target_link_all(${tmp} ${ODBC_LIBRARIES})
  else()
    wex_target_link_all(${tmp})
  endif()

  add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
  )
endfunction()

# general setup

if(WIN32)
  set(LOCALE_INSTALL_DIR bin)
else()
  set(LOCALE_INSTALL_DIR share/locale/)
endif()

if(MSVC)
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    /D_CRT_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_DEPRECATE
    /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS \
    /std:c++latest /Zc:__cplusplus"
  )

  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  endif()
else()
  if(CMAKE_BUILD_TYPE MATCHES "Coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
    set(
      CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fPIC \
      --param ggc-min-expand=3 --param ggc-min-heapsize=5120"
    )
  endif()

  if(CMAKE_BUILD_TYPE MATCHES "Profile")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  endif()

  if(CMAKE_BUILD_TYPE MATCHES "valgrind")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  endif()

  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -Wno-overloaded-virtual -Wno-reorder -Wno-write-strings \
    -Wno-deprecated-declarations -Wno-unused-result"
  )
endif()

get_filename_component(wexSETUP_DIR_H ${wexSETUP_H} DIRECTORY)
get_filename_component(wexSETUP_DIR_H ${wexSETUP_DIR_H} DIRECTORY)

list(
  APPEND
  wxTOOLKIT_INCLUDE_DIRS
  ${wexSETUP_DIR_H}
  include
  external/pugixml/src
  external/ctags/libreadtags/
  external/wxMaterialDesignArtProvider/MaterialDesign/
  external/wxWidgets/include
  external
)

foreach(arg ${wxTOOLKIT_INCLUDE_DIRS})
  include_directories(${arg})
endforeach()

list(APPEND wxTOOLKIT_DEFINITIONS HAVE_WCSLEN)

foreach(arg ${wxTOOLKIT_DEFINITIONS})
  add_definitions(-D${arg})
endforeach()
