wxExtension contains a wxWidgets extension library, adding xml lexer 
configuration and useful classes to wxWidgets, 
and some applications that show how to use it.

The <a href="http://antonvw.github.com/syncped/">syncped</a> application is 
one of these applications, being a full featured source code text editor. 

## General

- git repository 
  - on github https://github.com/antonvw/wxExtension  
  
  - wxWidgets 2.9.2 is used as a stable master branch
  - the mac-os branch is added because the master uses c++0x flags, not supported
    by gcc 4.0.1 under mac os 10.4

- old subversion repository 
  - on xp-dev.com http://svn.xp-dev.com/svn/wxextension/
  
  - access 
  The SVN support in wxExVCS works with SVN client 1.6.12
  http://www.sliksvn.com/en/download
  - under windows TortoiseSVN is used (TortoiseSVN 1.6.15)
  http://tortoisesvn.tigris.org/,
  - under Linux svn client (version 1.6.12 (r955767))
  - under mac os 10.4 svn client (version 1.5.5 http://homepage.mac.com/martinott/)
    
- Source code      
  - Coding standard:
  http://www.gnu.org/prep/standards/standards.html
  and wxWidgets guidelines
  http://www.wxwidgets.org/develop/standard.htm

  - STL is used whenever possible 
  
  - GUI development library wxWidgets 
  http://www.wxwidgets.org/ (use STL instead of wxWidgets containers)

  - database is OTL (Version 4.0.214)
  http://otl.sourceforge.net/

- icons and bitmaps
  menu and toolbar bitmaps are from wxWidgets, using wxArtProvider, 
  or gtk stock items
  application icons are from
  http://tango.freedesktop.org/Tango_Desktop_Project
  and converted to ico using
  http://www.convertico.com/
  and converted to xpm using GIMP (2.6.6)    
  http://www.gimp.org/
  and convert to mac icns (first make 128 by 128 icon),
  then use http://iconverticons.com/ to convert to mac icns.

- Documentation
  - API documentation is generated from the sources using Doxygen (1.7.1)
  http://www.stack.nl/~dimitri/doxygen/
  - Other docs are in html format using no special html editor

- Automated testing is done using cppunit (1.12)
  http://sourceforge.net/projects/cppunit
    sudo apt-get install libcppunit-dev
     
- Translation is done using poedit (1.4.6)
  http://www.poedit.net/
  wxextension has it's own localization file, your application should
  also add it's own one, and also put the standard wxwidgets localization file
  in the localization dir. 
  Currently a separate file wxstd-xxx-nl.po is added, as dutch translation
  is updated only for wxWidgets 2.8.0, whereas we use 2.9, in the added file
  the extra needed translation are put. 
  The place where to put your po files can be found by running wxex-sample,
  that shows the folder on the status bar. 
  You can also test other languages using the special LANG config item,
  e.g. setting it to 80 allows you to test french translation.

- Build process/IDE
  - First of all, the new C++ auto keyword is used a lot, so
  you need a recent compiler to compile sources.

  - Project and make files are generated using Bakefile 0.2.9
  http://www.bakefile.org/
  this is done in the build dir:
    
    - under windows using Microsoft Visual Studio 2010
    Version 10.0.30319.1 RTMRel
    using format msvs2008prj
    TODO: fix, this no longer works
      
    - under windows using command line prompt
    using format msvc
        nmake -f makefile.vc WXWIN=c:\wxwidgets-2.9.2
      
    - under Ubuntu 11.04 linux gcc gcc (Ubuntu/Linaro 4.5.2-8ubuntu4) 4.5.2
    (no IDE)
    using gtk version:
        ../configure --with-gtk
    GNUMakefile generated using format gnu
  
    - under SunOS using the Sun make gives errors,
    you have to use GNU make (/usr/sfw/bin)
    and the -mt option should be removed from generated Makefile, and
    option -w should be added
    this config command was used (socket file did not compile, and
    glcancas neither):
        ../configure --with-gtk --disable-sockets --without-opengl
    than libs were built, but listctrl and auidemo both crashed, no
    more attempts (wxWidgets 2.9.1).
  
    - under cygwin 1.7 wxWidgets 2.9.1 does not compile (snapshot does)
        ../configure --with-msw
    do a make and a make install
    Strangely, wxWidgets libs build, wxextension does not (wxcrt.h complains).
    Also g++ version (g++ (GCC) 4.3.4 20090804 (release) 1) does not support c++0x.
      
    - under mac os 10.4 use gcc 4.0.1 (part of xcode25_8m2258_developerdvd.dmg)
      ../configure --with-mac
  
  - debug under Windows using IDE, under Linux using Kdbg

  - deploy under Windows using 7-Zip 9.20
  strip the executable under Windows using UPX
  http://upx.sourceforge.net/
  http://www.7-zip.org
  deploy under Linux using deploy.sh (in build dir)


## When adding functionality

- apply a patch:
    patch -p0 -i fixes.patch

- document it in the source in doxy way

- if it needs to be translated, add _() around text strings, 
  and update po file(s)

- add a test for it in 
    extension/test/base, 
    extension/test/app,
    extension/test/report, where it has least dependencies,
  run test-all.sh from the build dir, which collects output in several log files, 
  and commits automatically.

- add a sample for it in
    extension/sample, 
    extension/sample/report
