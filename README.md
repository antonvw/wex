wxExtension contains a wxWidgets extension library, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Dependencies

- [wxWidgets 3.0](http://www.wxwidgets.org/)
  
- [cmake](http://www.cmake.org/) to generate makefiles   
  When using wxWidgets 3.1 you can clone wxWidgets to c:\wxWidgets.

- [catch lib 1.3.0](https://github.com/philsquared/Catch/) (only for testing, header only)   
    
- [OTL database 4.0.214](http://otl.sourceforge.net/) (for syncodbcquery, header only)   
    `sudo apt-get install unixODBC unixODBC-dev` or   
    `yum install unixODBC unixODBC-devel`  

# Build process 
  [![Travis](https://travis-ci.org/antonvw/wxExtension.png?branch=master)](https://travis-ci.org/antonvw/wxExtension)
  [![Appveyor](https://ci.appveyor.com/api/projects/status/x3jm519fq1i407a6?svg=true)](https://ci.appveyor.com/project/antonvw/wxextension)
  [![Coverity](https://scan.coverity.com/projects/2868/badge.svg)](https://scan.coverity.com/projects/2868>)
  [![Coveralls](https://coveralls.io/repos/antonvw/wxExtension/badge.svg?branch=master&service=github)](https://coveralls.io/github/antonvw/wxExtension?branch=master)   

## Building wxWidgets

- under windows:   
    -- using Microsoft Visual Studio 2015 in build/msw:    
    `nmake /f makefile.vc` or   
    `nmake /f makefile.vc BUILD=release`   
    -- using mingw in build/msw:
    `make -f makefile.gcc`    
    -- using cygwin 1.7.9:   
    in buildmsw (created):
    `../configure --with-msw --disable-shared && make`  
    
- under Linux g++ 4.9.2:   
    install gtk:   
    `sudo apt-get install libgtk2.0-dev`   or   
    `sudo apt-get install libgtk-3-dev`   
    then in buildgtk (created):   
    `../configure --with-gtk && make`  or   
    `../configure --with-gtk=3 && make`   and   
    `make install`    
    
- under Linux clang 3.5.0:   
    `export CC=clang`   
    `export CXX=clang++`    
    see g++   

## Building wxExtension        

- under windows:   
    `mkdir build`   
    `cd build`   
  -- using Visual Studio:   
    `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..`   
    `nmake`   
  -- using mingw:   
    `cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..`   
    `mingw32-make`   
  
  (or make a `debug` directory and use `-DCMAKE_BUILD_TYPE=Debug`)   
    
- under Linux:   
    `mkdir build`   
    `cd build`   
    `cmake ..`   
    `make`   

- for OTL add `-DwxExUSE_OTL=ON`    
