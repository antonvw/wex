wxExtension contains a wxWidgets extension library, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Requirements

- [cmake](http://www.cmake.org/)    
- a `c++17` standard supporting compiler    

## Building

```
git clone --recursive https://github.com/antonvw/wxExtension.git    
mkdir build && cd build   
cmake .. && make -DwxExBUILD_SP=ON   
```

for Visual Studio add `-G "NMake Makefiles"` and do `nmake`,
for mingw add `-G "MinGW Makefiles"` and do `mingw32-make`   

# Build process 

  [![Travis](https://travis-ci.org/antonvw/wxExtension.png?branch=master)](https://travis-ci.org/antonvw/wxExtension)
  [![Appveyor](https://ci.appveyor.com/api/projects/status/x3jm519fq1i407a6?svg=true)](https://ci.appveyor.com/project/antonvw/wxextension)
  [![Coveralls](https://coveralls.io/repos/antonvw/wxExtension/badge.svg?branch=master&service=github)](https://coveralls.io/github/antonvw/wxExtension?branch=master)   

# Uses

- [doctest lib](https://github.com/onqtam/doctest)    
- [easylogging++ lib](https://github.com/muflihun/easyloggingpp)    
- [eval lib](https://github.com/r-lyeh/eval)    
- [fsm lib](https://github.com/eglimi/cppfsm)    
- [OTL database lib](http://otl.sourceforge.net/)    
- [pugixml lib](https://github.com/zeux/pugixml)    
- [tclap lib](http://tclap.sourceforge.net/)    
- [universal-ctags lib](https://github.com/universal-ctags/ctags)    
- [wxWidgets lib](https://github.com/wxWidgets/wxWidgets/)
