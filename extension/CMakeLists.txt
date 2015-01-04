if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS /DwxExUSE_PORTABLE")

  if (CMAKE_BUILD_TYPE EQUAL "Debug")
    set(wxWidgets_CONFIGURATION mswud)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D__WXDEBUG__")
  else (CMAKE_BUILD_TYPE EQUAL "Debug")
    set(wxWidgets_CONFIGURATION mswu)
  endif (CMAKE_BUILD_TYPE EQUAL "Debug")
else (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wwrite-strings")
endif(WIN32)

if (WIN32)
  find_package(wxWidgets COMPONENTS scintilla aui stc html adv xml core base REQUIRED)
else (WIN32)
  find_package(wxWidgets COMPONENTS           aui stc html adv xml core base REQUIRED)
endif (WIN32)

include(
  ${wxWidgets_USE_FILE})

SET(CMAKE_INSTALL_PREFIX ..)

add_subdirectory(src)
add_subdirectory(src/report)
add_subdirectory(sample)
add_subdirectory(test)
