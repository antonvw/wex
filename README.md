wxExtension contains a wxWidgets extension library, adding xml lexer 
configuration and useful classes to wxWidgets, 
and some applications that show how to use it.

The [syncped](http://antonvw.github.com/syncped) application is 
one of these applications, being a full featured source code text editor. 

# Dependencies

- [wxWidgets 2.9.3](http://www.wxwidgets.org/) is used as a stable master branch  
  
- [OTL database](http://otl.sourceforge.net/) (Version 4.0.214)  
  

# Build process

## Building wxWidgets

- under windows using Microsoft Visual Studio 2010 nmake    
    `nmake -f makefile.vc` or
    `nmake -f makefile.vc BUILD=release`
    
- under windows using cygwin 1.7.9   
    `../configure --with-msw --disable-shared`  
    
- under Ubuntu 11.10 linux gcc (Ubuntu/Linaro 4.6.1-9ubuntu3) 4.6.1   
    `../configure --with-gtk`  

- under mac os 10.4 use gcc 4.0.1 (part of xcode25_8m2258_developerdvd.dmg)  
    `../configure --with-mac`
    
- under SunOS using GNU make (/usr/sfw/bin)  
    `../configure --with-gtk --disable-shared --without-opengl`  
  
## Building wxExtension      
      
- First of all, the new C++ auto keyword is used a lot, so
  you need a recent compiler to compile sources.

- Project and make files are generated using [Bakefile 0.2.9](http://www.bakefile.org/)  
  In the build dir:
  
  - under windows:  
    `make` or `make-release`
    
  - under cygwin   
    `make`  
    wxextension does not yet compile
    
  - under Ubuntu:  
    `make`
    
  - under mac:  
    `make -f GNUMakefile-mac`
    
  - under SunOS:  
    `/usr/sfw/bin/make`
  
# Adding functionality

- use STL whenever possible 

- icons and bitmaps
  - menu and toolbar bitmaps are from wxWidgets, using wxArtProvider

  - application icons from [Tango](http://tango.freedesktop.org/Tango_Desktop_Project),
  [converted to ico](http://www.convertico.com/), 
  [converted to xpm using GIMP (2.6.6)](http://www.gimp.org/), 
  [convert to mac icns (first make 128 by 128 icon)](http://iconverticons.com/)

- apply a patch:  
    `patch -p0 -i fixes.patch`

- doxy document sources  
  API documentation is generated from the sources 
  using [Doxygen (1.7.1)](http://www.stack.nl/~dimitri/doxygen/)

- translation is done using [poedit (1.4.6)](http://www.poedit.net/)    
  - wxextension has it's own localization file, your application should
    also add it's own one (add _() around text strings), 
    and also put the standard wxwidgets localization file
    in the localization dir.  

  - The place where to put your po files can be found by running wxex-sample,
    that shows the folder on the status bar.   
    You can also test other languages using the special LANG config item,
    e.g. setting it to 80 allows you to test french translation.

- automated testing is done using [cppunit (1.12)](http://sourceforge.net/projects/cppunit):   
    `sudo apt-get install libcppunit-dev`  
  run test-all.sh from the build dir, which collects output in several log files. 

# Deploy

- under Windows using deploy.bat (in build dir)

- under Linux using deploy.sh (in build dir)
