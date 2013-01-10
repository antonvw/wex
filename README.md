wxExtension contains a wxWidgets extension library, adding xml lexer 
configuration and useful classes to wxWidgets, 
and some applications that show how to use it.

The [syncped](http://antonvw.github.com/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Dependencies

- [wxWidgets 2.9.5](http://www.wxwidgets.org/)
  
- [OTL database 4.0.214](http://otl.sourceforge.net/) is used by syncodbcquery  
  

# Build process

## Building wxWidgets

- under windows:   
    using Microsoft Visual Studio 2012 nmake:    
    `nmake -f makefile.vc` or   
    `nmake -f makefile.vc BUILD=release`   
    using cygwin 1.7.9:   
    `../configure --with-msw --disable-shared`  
    
- under Ubuntu 12.10 g++ (Ubuntu/Linaro 4.7.2-2ubuntu1) 4.7.2:   
    install gtk:   
    `sudo apt-get install libgtk2.0-dev`   or   
    `sudo apt-get install libgtk-3-dev`   
    then:   
    `../configure --with-gtk`  or   
    `../configure --with-gtk=3`   
    
- under SunOS using GNU make (/usr/sfw/bin):  
    `../configure --with-gtk --disable-shared --without-opengl`  
  
- under mac os 10.4 use gcc 4.0.1 (use v2.9.3 tag) (part of xcode25_8m2258_developerdvd.dmg):   
    `../configure --with-mac`

## Building wxExtension (in the build dir)
  
- under windows:   
    using Microsoft Visual Studio 2012:   
    `make` or `make-release`   
    using cygwin:   
    `make`  
    wxExtension does not yet compile
    
- under Ubuntu:  
    `make`
    
- under SunOS:  
    `/usr/sfw/bin/make`   
    
- under mac:  
    `make -f GNUMakefile-mac`
    
