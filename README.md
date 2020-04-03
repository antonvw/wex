wex contains a library that offers c++ ex and vi functionality, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Requirements

- [cmake](http://www.cmake.org/)    
- [boost](https://www.boost.org) (do not use version 1.72)
- a `c++17` standard supporting compiler (clang-8 on osx)    

## Building

```
git clone --recursive https://github.com/antonvw/wex.git    
mkdir build && cd build   
cmake .. && make
```

For Visual Studio 2019 do 
  `devenv wex.sln /Build Release`,
for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`, for osx do 
`brew install llvm@8` and `. ci/use-clang.sh` before invoking cmake.

# Build process 

  [![Travis](https://travis-ci.org/antonvw/wex.png?branch=develop)](https://travis-ci.org/antonvw/wex)
  [![Appveyor](https://ci.appveyor.com/api/projects/status/a346d8537whyrjev?svg=true)](https://ci.appveyor.com/project/antonvw/wex)
  [![Coveralls](https://coveralls.io/repos/antonvw/wex/badge.svg?branch=develop&service=github)](https://coveralls.io/github/antonvw/wex?branch=develop)   

# Uses

- [doctest lib](https://github.com/onqtam/doctest)    
- [easylogging++ lib](https://github.com/muflihun/easyloggingpp)    
- [json lib](https://github.com/nlohmann/json)    
- [OTL database lib](http://otl.sourceforge.net/)    
- [pugixml lib](https://github.com/zeux/pugixml)    
- [universal-ctags lib](https://github.com/universal-ctags/ctags)    
- [wxWidgets lib](https://github.com/wxWidgets/wxWidgets/)
