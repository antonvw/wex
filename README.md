# wex library

wex contains a library that offers c++ ex and vi functionality.

The [syncped](http://sourceforge.net/projects/syncped) application
shows a usage of this library, offering a full featured source code text editor.

## Requirements

- [cmake](http://www.cmake.org/)
- [boost](https://www.boost.org)
- a `c++23` standard supporting compiler
- for Linux or osx [ninja](https://ninja-build.org) is optional

## Building

```bash
git clone --recursive https://github.com/antonvw/wex.git
```

### for linux or osx

`./build-gen.sh`

### for Visual Studio

```bash
mkdir build && cd build && cmake ..
devenv wex.sln /build Release
```

and for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`.

## Use wex lib in your own application

install wex
(on windows as administrator `cmake.exe -P cmake_install.cmake`)
and do `find_package(WEX)` in your CMakeLists.txt. This will provide the
`wex_FOUND`, `wex_INCLUDE_DIR`, `wex_LIB_DIR` and `wex_LIBRARIES` variables.
An example is the [syncped editor](https://gitlab.kitware.com/antonvw/syncped).

## Build process

  [![Appveyor](https://ci.appveyor.com/api/projects/status/a346d8537whyrjev?svg=true)](https://ci.appveyor.com/project/antonvw/wex)
  [![Codacy Badge](https://app.codacy.com/project/badge/Grade/2fcaabd94e984dfc97740fe9f53472f5)](https://app.codacy.com/gh/antonvw/wex/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
  [![Coverage Status](https://coveralls.io/repos/github/antonvw/wex/badge.svg?branch=develop)](https://coveralls.io/github/antonvw/wex?branch=develop)

## Uses

- [doctest lib](https://github.com/doctest/doctest)
- [MaterialDesignArtProvider](https://github.com/perazz/wxMaterialDesignArtProvider)
- [OTL database lib](http://otl.sourceforge.net/)
- [pugixml lib](https://github.com/zeux/pugixml)
- [universal-ctags lib](https://github.com/universal-ctags/ctags)
- [wxWidgets lib](https://github.com/wxWidgets/wxWidgets/)
