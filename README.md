wex contains a library that offers c++ ex and vi functionality, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Requirements

- [cmake](http://www.cmake.org/)    
- [boost](https://www.boost.org)
- a `c++17` standard supporting compiler (clang-8 on osx)    

## Building

```
git clone --recursive https://github.com/antonvw/wxExtension.git    
mkdir build && cd build   
cmake .. && make
```

for Visual Studio add `set CL=/Zc:__cplusplus` before invoking cmake and do 
  `devenv wex.sln`,
for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`   

# Build process 

  [![Travis](https://travis-ci.org/antonvw/wxExtension.png?branch=master)](https://travis-ci.org/antonvw/wxExtension)
  [![Appveyor](https://ci.appveyor.com/api/projects/status/x3jm519fq1i407a6?svg=true)](https://ci.appveyor.com/project/antonvw/wxExtension)
  [![Coveralls](https://coveralls.io/repos/antonvw/wxExtension/badge.svg?branch=master&service=github)](https://coveralls.io/github/antonvw/wxExtension?branch=master)   

# Uses

- [doctest lib](https://github.com/onqtam/doctest)    
- [easylogging++ lib](https://github.com/muflihun/easyloggingpp)    
- [OTL database lib](http://otl.sourceforge.net/)    
- [pugixml lib](https://github.com/zeux/pugixml)    
- [tclap lib](http://tclap.sourceforge.net/)    
- [universal-ctags lib](https://github.com/universal-ctags/ctags)    
- [wxWidgets lib](https://github.com/wxWidgets/wxWidgets/)
