# wex library

wex contains a library that offers c++ ex and vi functionality.

The [syncped](http://sourceforge.net/projects/syncped) application
shows a usage of this library, offering a full featured source code text editor.

## Requirements

- [cmake](http://www.cmake.org/)
- [boost](https://www.boost.org)
- a `c++20` standard supporting compiler

## Building

```bash
git clone --recursive https://github.com/antonvw/wex.git
mkdir build && cd build
cmake .. && make && make install
```

For Visual Studio 2019 do
  `devenv wex.sln /build Release`,
for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`.

If you would like to use shared libs for Boost, wxWidgets and wex add
`-DwexBUILD_SHARED=ON`.

To use wex lib in your own application do `make install`
(on windows as administrator `cmake.exe -P cmake_install.cmake`)
and do `find_package(WEX)` in your CMakeLists.txt. This will provide the
`wex_FOUND`, `wex_INCLUDE_DIR`, `wex_LIB_DIR` and `wex_LIBRARIES` variables.

## Build process

  [![Appveyor](https://ci.appveyor.com/api/projects/status/a346d8537whyrjev?svg=true)](https://ci.appveyor.com/project/antonvw/wex)

## Uses

- [doctest lib](https://github.com/doctest/doctest)
- [OTL database lib](http://otl.sourceforge.net/)
- [pugixml lib](https://github.com/zeux/pugixml)
- [universal-ctags lib](https://github.com/universal-ctags/ctags)
- [wxWidgets lib](https://github.com/wxWidgets/wxWidgets/)
