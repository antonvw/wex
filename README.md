wxExtension contains a wxWidgets extension library, 
and some applications that show how to use it.

The [syncped](http://sourceforge.net/projects/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Dependencies

- [wxWidgets 3.0](http://www.wxwidgets.org/)
  
- [cppunit 1.12](http://sourceforge.net/projects/cppunit) (only for testing)   
    `sudo apt-get install libcppunit-dev` or   
    `yum install cppunit-devel`  
    
- [OTL database 4.0.214](http://otl.sourceforge.net/) is used by syncodbcquery  
    `sudo apt-get install unixODBC unixODBC-dev` or   
    `yum install unixODBC`  
    `yum install unixODBC-devel`  

- [cmake](http://www.cmake.org/) to generate makefiles

# Build process 
  [![Build Status](https://travis-ci.org/antonvw/wxExtension.png?branch=v3.0)](https://travis-ci.org/antonvw/wxExtension) 
  [![Coverity Scan Build Status](https://scan.coverity.com/projects/2868/badge.svg)](https://scan.coverity.com/projects/2868>)
  [![Coverage Status](https://img.shields.io/coveralls/antonvw/wxExtension.svg)](https://coveralls.io/r/antonvw/wxExtension?branch=v3.0)  
  
## Building wxWidgets

- under windows:   
    using Microsoft Visual Studio 2013 nmake in build/msw:    
    `nmake -f makefile.vc` or   
    `nmake -f makefile.vc BUILD=release`   
    using cygwin 1.7.9:   
    in buildmsw (created):
    `../configure --with-msw --disable-shared && make`  
    
- under Linux g++ 4.7.3:   
    install gtk:   
    `sudo apt-get install libgtk2.0-dev`   or   
    `sudo apt-get install libgtk-3-dev`   
    then in buildgtk (created):   
    `../configure --with-gtk && make`  or   
    `../configure --with-gtk=3 && make`   and   
    `make install`    
    
- under SunOS:  
    `../configure --with-gtk --disable-shared --without-opengl --disable-mediactrl && make`  
  
- under mac os 10.4 use gcc 4.0.1 (use v2.9.3 tag) (part of xcode25_8m2258_developerdvd.dmg):   
    `../configure --with-mac && make`

## Building wxExtension        
    
- under windows:   
    `cd build`   
    `cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..`   
    `nmake`   
  (or make a `Debug` directory and use `-DCMAKE_BUILD_TYPE=Debug`)   
    
- under Linux:   
    `cd build`    
    `cmake -DCMAKE_BUILD_TYPE=Release ..`   
    `make`   

- for OTL add `-DwxExUSE_OTL=ON`    
