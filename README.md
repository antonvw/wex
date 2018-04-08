wxExtension contains a wxWidgets extension library, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Dependencies

- [wxWidgets 3.x](http://www.wxwidgets.org/) or [this fork](https://github.com/antonvw/wxWidgets/)
  
- [cmake](http://www.cmake.org/)    

# Uses

- [doctest lib](https://github.com/onqtam/doctest)    

- [easylogging++ lib](https://github.com/muflihun/easyloggingpp)    

- [eval lib](https://github.com/r-lyeh/eval)    

- [fsm lib](https://github.com/eglimi/cppfsm)    

- [OTL database lib](http://otl.sourceforge.net/)    

- [pugixml lib](https://github.com/zeux/pugixml)    

- [tclap lib](http://tclap.sourceforge.net/)    

- [universal-ctags lib](https://github.com/universal-ctags/ctags)    

# Build process 

  [![Travis](https://travis-ci.org/antonvw/wxExtension.png?branch=master)](https://travis-ci.org/antonvw/wxExtension)
  [![Appveyor](https://ci.appveyor.com/api/projects/status/x3jm519fq1i407a6?svg=true)](https://ci.appveyor.com/project/antonvw/wxextension)
  [![Coveralls](https://coveralls.io/repos/antonvw/wxExtension/badge.svg?branch=master&service=github)](https://coveralls.io/github/antonvw/wxExtension?branch=master)   

## Building wxWidgets

- under windows:   
    -- using Microsoft Visual Studio 2017:    
    `git clone --recursive`   
    `mkdir build`   
    `cd build`   
    
    (or make a `debug` directory and use `-DCMAKE_BUILD_TYPE=Debug`)   
    
    `cmake.exe -DwxBUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=Release ..`    
    `nmake`    
    copy generated lib\vc_lib to wxWidgets\lib
    
- under Linux g++ 7.1.1:   
    install gtk:   
    `sudo apt-get install libgtk2.0-dev` or   
    `sudo apt-get install libgtk-3-dev`   
    then in buildgtk (created):   
    `../configure --with-gtk && make` or   
    `../configure --with-gtk=3 && make` and   
    `make install`    
    
- under Linux clang 3.5.0:   
    `export CC=clang`   
    `export CXX=clang++`    
    see g++   
    
- under MacOS:    
    `brew install wxwidgets`    
    `brew install xcode`    

## Building wxExtension        

- `git clone --recursive`   
  `mkdir build`   
  `cd build`   

- under windows:   
  -- using Visual Studio:   
    `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..`   
    `nmake`   
  -- using mingw:   
    `cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..`   
    `mingw32-make`   
  
- under Linux or MacOS:   
    `cmake ..`   
    `make`   

- for OTL add `-DwxExUSE_OTL=ON`    
