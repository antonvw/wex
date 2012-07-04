echo off
rem Name:      make-release.bat
rem Purpose:   Batch file to run nmake to make a release version
rem Author:    Anton van Wezenbeek
rem Copyright: (c) 2012 Anton van Wezenbeek

nmake -f makefile-release.vc WXWIN=c:\wxwidgets-2.9.4
